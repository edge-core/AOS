include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libdbsync_private.a
lib_LTLIBRARIES = libdbsync.la

libdbsync_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/dbsync_txt_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/dbsync_txt_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/dbsync_txt_init.c

libdbsync_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
    -I$(ACCTONSRC_DIR)/user/process/xfer_proc/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/prom/diag/network_adpt/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include

libdbsync_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/dbsync_txt_pmgr.c

libdbsync_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dbsync/txt/include
    

CFLAGS += -Werror 

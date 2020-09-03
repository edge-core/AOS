include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsysmgmt_private.a
lib_LTLIBRARIES=libsysmgmt.la

libsysmgmt_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_bnr_pmgr.c 	

libsysmgmt_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sysmgmt_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sysmgmt_task.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_env_vm.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_bnr_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_reload_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sys_reload_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/sysmgmt_backdoor.c
	
libsysmgmt_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include		

libsysmgmt_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(OEM_INCLUDEDIR) \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
 	-I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/user/driver/fs/include \
	-I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include  \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include\
	-I$(ACCTONSRC_DIR)/user/process/sys_mgmt_proc/include

CFLAGS += -Werror 

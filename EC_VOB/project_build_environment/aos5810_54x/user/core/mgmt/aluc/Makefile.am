include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libaluc_private.a


libaluc_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client_doc.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client_file_watch.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client_log.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client_storage.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/alu_client_time.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/aluc/json_object_path.c

libaluc_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/alu/build/include \
	-I$(ACCTONSRC_DIR)/alu/build/include/deps \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/aluc \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/driver/fs/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I${ACCTONSRC_DIR}/user/thirdpty/jansson/src

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libxfer_private.a
lib_LTLIBRARIES = libxfer.la


libxfer_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/xfer_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/xfer_dnld.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/xfer_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/tftp.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/ftp/cmdftp.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/xfer_mgr.c

libxfer_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/ftp \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
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
	-I$(ACCTONSRC_DIR)/user/process/xfer_proc/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include          \
    -I$(ACCTONSRC_DIR)/user/thirdpty/inih \
	-I$(BOOTLOADER_DIR)/include \
	-I$(BOOTLOADER_DIR)/drivers/ams/partition

libxfer_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/xfer/xfer_pmgr.c

libxfer_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
	-I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/fs/include
CFLAGS += -Werror -Wno-misleading-indentation

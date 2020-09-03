include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libvxlan_private.a

lib_LTLIBRARIES = libvxlan.la


libvxlan_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/vxlan/vxlan_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l3/vxlan/vxlan_mgr.c


libvxlan_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/protocol \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include


libvxlan_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/vxlan/vxlan_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l3/vxlan/vxlan_pom.c \
	$(ACCTONSRC_DIR)/user/core/l3/vxlan/vxlan_om.c

libvxlan_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
  -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include

AM_CFLAGS = -Werror

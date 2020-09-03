include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libudphelper_private.a

lib_LTLIBRARIES = libudphelper.la


libudphelper_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/include/udphelper_init.h \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/include/udphelper_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/include/udphelper_om.h \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/include/udphelper_vm.h \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/include/udphelper_type.h \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_init.c \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_om.c \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_vm.c

libudphelper_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)//user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/cmnlib/common/include\
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants\
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


libudphelper_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/udphelper/udphelper_pmgr.c

libudphelper_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include

AM_CFLAGS = -Werror

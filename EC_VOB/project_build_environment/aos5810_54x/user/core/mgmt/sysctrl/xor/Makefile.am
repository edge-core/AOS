include $(top_builddir)/commdef.mk


lib_LTLIBRARIES = libsysctrl_xor.la

libsysctrl_xor_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include/sysctrl_xor_mgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/sysctrl_xor_mgr.c

libsysctrl_xor_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/mlag/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include

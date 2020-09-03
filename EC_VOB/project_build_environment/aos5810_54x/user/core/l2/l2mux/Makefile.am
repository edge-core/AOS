include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libl2mux_private.a
lib_LTLIBRARIES = libl2mux.la

libl2mux_private_a_SOURCES=\
        $(ACCTONSRC_DIR)/user/core/l2/l2mux/l2mux_init.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mux/l2mux_mgr.c

libl2mux_private_a_CFLAGS=$(CFLAGS)

libl2mux_private_a_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include

libl2mux_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/l2mux/l2mux_pmgr.c

libl2mux_la_CPPFLAGS = \
          $(COMMON_CPPFLAGS) \
          -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include

AM_CFLAGS = -Werror

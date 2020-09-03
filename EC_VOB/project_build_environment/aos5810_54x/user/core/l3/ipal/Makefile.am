include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libipal.la

libipal_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_ioctl.c   \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_ioctl.h  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_kernel.c  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_route.c \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_rt_netlink.c  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_rt_netlink.h  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_sysctl.c  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_sysctl.h  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_vrrp.c  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_if.c  \
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_neigh.c	\
	$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_icmp.c

libipal_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include  \
	-I$(ACCTONSRC_DIR)/cmnlib/common/include    \
	-I$(ACCTONSRC_DIR)/kernel/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/protocol/IPI_7.4.0.t40_accton/ZebOS5/pal/api    \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include    \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include    \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include 	

CFLAGS += -Werror

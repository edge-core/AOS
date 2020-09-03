include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libpim.la

libpim_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/pim/pim_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/l3/pim/pim_pom.c \
	$(ACCTONSRC_DIR)/user/core/l3/pim/pim6_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/l3/pim/pim6_pom.c

libpim_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/l3/pim/include \
        -I$(ACCTONSRC_DIR)/user/protocol/IPI_7.4.0.t40_accton/ZebOS5/pimd \
        -I$(ACCTONSRC_DIR)/user/protocol/IPI_7.4.0.t40_accton/ZebOS5/pdmd \
        -I$(ACCTONSRC_DIR)/user/protocol/IPI_7.4.0.t40_accton/ZebOS5/pim6d \
        -I$(ACCTONSRC_DIR)/user/protocol/IPI_7.4.0.t40_accton/ZebOS5/nsm

CFLAGS += -Werror

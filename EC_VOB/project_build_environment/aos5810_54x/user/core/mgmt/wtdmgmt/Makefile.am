include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libwtdmgmt.la

libwtdmgmt_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/sw_watchdog_mgr.c 

libwtdmgmt_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include
    
CFLAGS += -Werror 

include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libmsl_private.a
lib_LTLIBRARIES=libmsl.la

libmsl_private_a_SOURCES=\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_task.c\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_intf.c\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_fsm.c\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_drv.c\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_mgr.c\
	$(ACCTONSRC_DIR)/user/driver/msl/mslt/msl_backdoor.c 

libmsl_private_a_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/process/driver_proc/include\
    -I$(ACCTONSRC_DIR)/user/driver/msl/include \
    -I$(ACCTONSRC_DIR)/user/driver/msl/mslt/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include

libmsl_la_SOURCES=\
	$(ACCTONSRC_DIR)/user/driver/msl/msl_pmgr.c

libmsl_la_CPPFLAGS= \
	-I$(OEM_INCLUDEDIR) \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
  -I$(ACCTONSRC_DIR)/cmnlib/common/include \
  -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
  -I$(ACCTONSRC_DIR)/user/process/driver_proc/include \
  -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/msl/mslt/include \
  -I$(ACCTONSRC_DIR)/user/driver/msl/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format


lib_LTLIBRARIES=libsystime.la
noinst_LIBRARIES=libsystime_private.a


libsystime_la_SOURCES=$(ACCTONSRC_DIR)/user/driver/systime/sys_time.c \
                     $(ACCTONSRC_DIR)/user/driver/systime/sys_time_init.c  
libsystime_la_CPPFLAGS= \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include  \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
    -D__SECONDS1970__=`date +%s`

libsystime_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/driver/systime/sys_time_task.c
libsystime_private_a_CPPFLAGS= \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include  \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include

AM_CFLAGS = -Werror

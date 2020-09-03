lib_LTLIBRARIES = libfs.la
noinst_LIBRARIES = libfs_static.a

all-local:
	test -d utest || mkdir utest
	cd utest && make -f $(ACCTONSRC_DIR)/user/driver/fs/utest/Makefile run_fs_utest

clean-local:
	@rm -rf utest >&/dev/null

libfs_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/driver/fs/fs.c \
	$(ACCTONSRC_DIR)/user/driver/fs/fs_init.c \
	$(ACCTONSRC_DIR)/user/driver/fs/fs_om.c
 
libfs_la_CFLAGS=$(CFLAGS)
	
libfs_la_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/driver/fs/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
        -I$(ACCTONSRC_DIR)/sysinclude \
        -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
        -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
        -I$(ACCTONSRC_DIR)/cmnlib/common/include \
        -I$(ACCTONSRC_DIR)/cmnlib/user/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include

libfs_static_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/driver/fs/fs_static_lib.c

libfs_static_a_CFLAGS=$(CFLAGS)

libfs_static_a_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/driver/fs/include \
        -I$(ACCTONSRC_DIR)/sysinclude \
        -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
        -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
        -I$(ACCTONSRC_DIR)/cmnlib/common/include \
        -I$(ACCTONSRC_DIR)/cmnlib/user/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include

CFLAGS += -Werror

VPATH=$(ACCSRC)/user/driver/fs:$(ACCSRC)/user/driver/fs/utest
CC=gcc
SRCS=fs.c fs_init.c

OBJS=$(patsubst %.c, %.o, $(SRCS))

INCLUDE_PATHS=-I$(ACCSRC)/user/driver/fs/include \
	-I$(ACCSRC)/user/driver/fs \
        -I$(ACCSRC)/user/core/mgmt/stkmgmt/include \
        -I$(ACCSRC)/user/core/mgmt/ucmgmt/include \
        -I$(ACCSRC)/user/driver/devdrv/include \
        -I$(ACCSRC)/user/driver/sysdrv/include \
        -I$(ACCSRC)/sysinclude \
        -I$(ACCSRC)/sysinclude/oem/$(PROJECT_NAME) \
        -I$(ACCSRC)/sysinclude/mibconstants \
        -I$(ACCSRC)/cmnlib/common/include \
        -I$(ACCSRC)/cmnlib/user/include \
        -I$(ACCSRC)/user/core/mgmt/sysrsc/include \
        -I$(ACCSRC)/user/driver/iscdrv/include \
        -I$(ACCSRC)/user/driver/buffermgmt/include \
        -I$(ACCSRC)/user/driver/landrv/include \
        -I$(ACCSRC)/user/apps/backdoor/include \
        -I$(ACCSRC)/user/core/mgmt/wtdmgmt/include

CFLAGS=-g -D$(D_MODEL_NAME) $(INCLUDE_PATHS)

all: libfs.a fs_utest

clean:
	@rm core* *.o *.a 2>/dev/null || true
	@rm fs_utest 2>/dev/null || true

libfs.a: $(OBJS)
	ar rs $@ $<

libfs_utest_dummy.a: fs_utest_dummy_fun.o
	ar rs $@ $<

libfs_utest_mock.a: fs_utest_mock_om.o
	ar rs $@ $<

fs_utest: fs_utest.c libfs_utest_dummy.a libfs_utest_mock.a
	$(CC) -g -O0 $(CFLAGS) $^ -o fs_utest

run_fs_utest: fs_utest
	./fs_utest

PHONY: all clean

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@


VPATH=$(ACCSRC)/user/core/mgmt/xfer:$(ACCSRC)/user/core/mgmt/xfer/utest
CC=gcc
SRCS=tftp.c

OBJS=$(patsubst %.c, %.o, $(SRCS))

INCLUDE_PATHS=-I$(ACCSRC)/user/core/mgmt/xfer \
        -I$(ACCSRC)/sysinclude \
        -I$(ACCSRC)/sysinclude/oem/$(PROJECT_NAME) \
        -I$(ACCSRC)/sysinclude/mibconstants \
        -I$(ACCSRC)/cmnlib/common/include \
        -I$(ACCSRC)/cmnlib/user/include \
        -I$(ACCSRC)/user/apps/backdoor/include

CFLAGS=-g -D$(D_MODEL_NAME) $(INCLUDE_PATHS)

all: libtftp.a tftp_utest

clean:
	@rm core* *.o *.a 2>/dev/null || true
	@rm fs_utest 2>/dev/null || true

libtftp.a: $(OBJS)
	ar rs $@ $<

libtftp_utest_dummy.a: tftp_utest_dummy_fun.o
	ar rs $@ $<

tftp_utest: tftp_utest.c libtftp_utest_dummy.a
	$(CC) -g -O0 $(CFLAGS) $^ -o tftp_utest

run_tftp_utest: tftp_utest
	./tftp_utest

PHONY: all clean

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@


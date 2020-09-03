#
# This make job requires the following environment variables to be set:
#
# SDK                - path to StrataXGS SDK root directory
#
# Optionally the following environment variables can be set to
# override the default build server configuration:
#
# PPC_TOOLS_DIR      - path to build tools (if not in PATH already)
# PPC_CROSS_COMPILE  - cross compile tools prefix
# LINUX_INCLUDE      - path to Linux kernel include directory
#

relativepath = $(shell perl -MFile::Spec -e 'print File::Spec->abs2rel("'$(2)'","'$(1)'")')

KMODULE_OUTPUT = $(ACCROOTFS)/lib/modules/4.19.67

#
# for Make.linux
#
kernel_version = 4_19
platform := $(SDK_MODEL_NAME)
LINUX_MAKE_USER = 1
LINUX_MAKE_FLAGS := SDKBUILD=$(call relativepath,$(SDK),$(CURDIR)/build) OPT_CFLAGS="-Wno-error -fPIC"
LINUX_MAKE_SHARED_LIB = 0

#
# for Make.config
#
export MAKE_LOCAL := $(CURDIR)/Make.local

#
# for systems/linux/user/common/Makefile
#  or systems/linux/kernel/common/Makefile
#
BDE_BLDDIR := $(CURDIR)/build
ifdef LINUX_MAKE_USER
export DEST_DIR := $(BDE_BLDDIR)
else
export DESTDIR := $(BDE_BLDDIR)
endif

ADD_TO_CFLAGS = -Wno-error -fno-pie -std=gnu89
export ADD_TO_CFLAGS

export NO_LOCAL_TARGETS := 1

#
# to avoid undefined reference
#
ifneq ($(LINUX_MAKE_SHARED_LIB),1)
LIB_BCMSDK = build/unix-user/$(SDK_MODEL_NAME)/libbcmsdk.a
CMD_FIND_LIB_BCMSDK_OBJS = find $(dir $(LIB_BCMSDK)) -name '*.o' 2> /dev/null
endif

.PHONY: all build-sdk clean-lib make-lib install uninstall install-diag uninstall-diag show-cfgs

all: $(SDK)/Makefile build-sdk make-lib

build-sdk: $(MAKE_LOCAL)
	$(MAKE) MAKE_LOCAL=$(MAKE_LOCAL) build

install:
	mkdir -p $(KMODULE_OUTPUT)
	cp -f $(BDE_BLDDIR)/*-bde.ko $(KMODULE_OUTPUT)

uninstall:
	rm -f $(KMODULE_OUTPUT)/*-bde.ko

install-diag:
	cp -f build/bcm.user ${ACCROOTFS}/usr

uninstall-diag:
	rm -f build/bcm.user ${ACCROOTFS}/usr/bcm.user

show-cfgs:
	@echo CFGFLAGS=$(CFGFLAGS)

clean-lib:
	rm -f $(LIB_BCMSDK)

make-lib: clean-lib
ifdef LIB_BCMSDK
	$(CMD_FIND_LIB_BCMSDK_OBJS) | xargs ${CROSS_COMPILE}ar rcs $(LIB_BCMSDK)
	${CROSS_COMPILE}objcopy -N main $(LIB_BCMSDK)
endif

#
# git
#
ifeq ($(wildcard $(SDK)/Makefile),)
ifeq ($(shell which git),)
$(error Exit Docker and execute "make -C $$SDKPROJ git-checkout" first)
endif
endif

git-reference-submodule = $(shell git config --get-regexp "^submodule\..*\.url" "http://swpms\.accton\.com\.tw/git/aos-bcmsdk-zz" | head -1 | sed -e 's!^submodule\.\|\.url.*!!g')
git-reference-repo = $(addprefix ../../modules/,$(git-reference-submodule))
git-reference-opt = $(addprefix --reference ,$(git-reference-repo))

.PHONY: git-checkout

$(SDK)/Makefile:
	$(MAKE) git-checkout

git-checkout:
	cd `git rev-parse --show-toplevel` && git submodule update --init $(git-reference-opt) $(SDK)

#
# include SDK Makefiles
#
-include $(SDK)/make/Make.config
-include $(SDK)/make/Make.linux


SUBDIRS = as5812_54x as5812_54t as6812_32x

noinst_LIBRARIES=libdevdrvonlp_static.a
lib_LTLIBRARIES=libdevdrvonlp.la

libdevdrvonlp_static_a_SOURCES= \
    $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlpdrv_thermal.c \
    $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlpdrv_fan.c \
    $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlpdrv_psu.c

ONLPLIB_SRC_TOP_DIR=$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_v1
ONLPLIB_INCLUDE = \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/onlp/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/submodules/infra/modules/AIM/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/onlplib/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/bigcode/modules/IOF/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/infra/modules/AIM/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/sff/module/inc


libdevdrvonlp_static_a_CPPFLAGS= \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/include \
    $(ONLPLIB_INCLUDE)

libdevdrvonlp_la_SOURCES= \
    $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlpdrv_sfp.c

libdevdrvonlp_la_CPPFLAGS= \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/include


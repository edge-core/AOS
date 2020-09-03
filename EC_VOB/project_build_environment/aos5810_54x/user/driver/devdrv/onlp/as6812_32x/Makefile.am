include $(top_builddir)/commdef.mk

sbin_PROGRAMS = onlp_platmgrd_as6812_32x
bin_PROGRAMS = onlp_query_as6812_32x onlp_fanutil_as6812_32x onlp_sfputil_as6812_32x \
               onlp_psuutil_as6812_32x onlp_thermalutil_as6812_32x onlp_ledutil_as6812_32x

ONLPLIB_SRC_TOP_DIR=$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_v1
ONLPLIB_INCLUDE = \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/onlp/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/submodules/infra/modules/AIM/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/onlplib/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/bigcode/modules/IOF/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/infra/modules/AIM/module/inc \
    -I$(ONLPLIB_SRC_TOP_DIR)/submodules/onlp/modules/sff/module/inc


ONLPLIB_DIR=$(top_builddir)/driver/devdrv/onlp/onlp-accton
ONLPLIBS= \
    $(ONLPLIB_DIR)/onlp.a \
    $(ONLPLIB_DIR)/AIM.a \
    $(ONLPLIB_DIR)/x86_64_accton_as6812_32x.a \
    $(ONLPLIB_DIR)/onlplib.a \
    $(ONLPLIB_DIR)/onlp_platform_defaults.a \
    $(ONLPLIB_DIR)/IOF.a \
    $(ONLPLIB_DIR)/cjson_util.a \
    $(ONLPLIB_DIR)/cjson.a \
    $(ONLPLIB_DIR)/OS.a


onlp_query_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_query.c

onlp_query_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_query_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

onlp_fanutil_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_fanutil.c

onlp_fanutil_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_fanutil_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

onlp_sfputil_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_sfputil.c

onlp_sfputil_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_sfputil_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

onlp_psuutil_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_psuutil.c

onlp_psuutil_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_psuutil_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

onlp_thermalutil_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_thermalutil.c

onlp_thermalutil_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_thermalutil_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

onlp_platmgrd_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_platmgrd.c

onlp_platmgrd_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_platmgrd_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        $(ONLPLIB_DIR)/timer_wheel.a \
        -lm -lpthread -lrt

onlp_ledutil_as6812_32x_SOURCES = $(ACCTONSRC_DIR)/user/driver/devdrv/onlp/onlp_ledutil.c

onlp_ledutil_as6812_32x_CPPFLAGS = \
        $(ONLPLIB_INCLUDE)

onlp_ledutil_as6812_32x_LDADD = \
        $(ONLPLIBS) \
        -lm -lpthread -lrt

OEM_INCLUDEDIR=$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME)

COMMON_CPPFLAGS= \
    -I$(OEM_INCLUDEDIR) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/cmgr/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/debug/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(PROJECT_PATH)/user/thirdpty/openssl/include \
    -I$(PROJECT_PATH)/user/thirdpty/pcre2/out/include \
    -I${ACCTONSRC_DIR}/user/thirdpty/jansson/src \
    -I$(PROJECT_PATH)/user/thirdpty/libbson-1.1.10/include/libbson-1.0

COMMON_LIBS = \
    $(ACCPROJ)/cmnlib/libcmnlib.la \
    $(ACCPROJ)/user/apps/backdoor/libbackdoor.la \
    $(ACCPROJ)/user/apps/cli/libcli.la \
    $(ACCPROJ)/user/core/cmgr/libcmgr.la \
    $(ACCPROJ)/user/core/l2/lldp/liblldp.la \
    $(ACCPROJ)/user/core/l2/swctrl/lacp/liblacp.la \
    $(ACCPROJ)/user/core/l2/swctrl/rspan/librspan.la \
    $(ACCPROJ)/user/core/l2/swctrl/libnmtr.la \
    $(ACCPROJ)/user/core/l2/vlan/libvlan.la \
    $(ACCPROJ)/user/core/l3/netcfg/libnetcfg.la \
    $(ACCPROJ)/user/core/mgmt/cfgdb/libcfgdb.la \
    $(ACCPROJ)/user/core/mgmt/debug/libdebug.la \
    $(ACCPROJ)/user/core/mgmt/stkmgmt/libstkmgmt.la \
    $(ACCPROJ)/user/core/mgmt/sys_callback/libsys_callback.la \
    $(ACCPROJ)/user/core/mgmt/sysctrl/xor/libsysctrl_xor.la \
    $(ACCPROJ)/user/core/mgmt/sflow/libsflow.la \
    $(ACCPROJ)/user/core/mgmt/http/libhttp.la \
    $(ACCPROJ)/user/core/mgmt/ucmgmt/libucmgmt.la \
    $(ACCPROJ)/user/core/mgmt/wtdmgmt/libwtdmgmt.la \
    $(ACCPROJ)/user/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(ACCPROJ)/user/core/security/webauth/libwebauth.la \
    $(ACCPROJ)/user/driver/buffermgmt/libbuffermgmt.la \
    $(ACCPROJ)/user/driver/devdrv/libdevdrv.la \
    $(ACCPROJ)/user/driver/fs/libfs.la \
    $(ACCPROJ)/user/driver/iscdrv/libiscdrv.la \
    $(ACCPROJ)/user/driver/landrv/liblandrv.la \
    $(ACCPROJ)/user/driver/leddrv/libleddrv.la \
    $(ACCPROJ)/user/driver/swdrv/libswdrv.la \
    $(ACCPROJ)/user/driver/sysdrv/libsysdrv.la \
    $(ACCPROJ)/user/driver/devdrv/onlp/libdevdrvonlp.la \
    $(ACCPROJ)/user/driver/systime/libsystime.la \
    $(ACCPROJ)/user/thirdpty/openssl/libcrypto.so \
    $(ACCPROJ)/user/thirdpty/openssl/libssl.so \
    $(ACCPROJ)/user/thirdpty/pcre2/out/lib/libpcre2-8.la \
    $(ACCPROJ)/user/thirdpty/jansson/.libs/libjansson.so.0.0.0 \
    $(ACCPROJ)/user/thirdpty/libbson-1.1.10/lib/libbson-1.0.la \
    $(ACCPROJ)/user/core/l2/cn/libcn.la \
    $(ACCPROJ)/user/core/l2/mlag/libmlag.la \
    $(ACCPROJ)/user/core/l2/pfc/libpfc.la \
    $(ACCPROJ)/user/core/l3/vxlan/libvxlan.la \
    $(ACCPROJ)/user/core/l3/amtrl3/libamtrl3.la \
    $(SDK_LIBS)

#
# for SDK
#
SDK_CPPFLAGS = \
    -DBROADCOM_DEBUG \
    $(BCM_CFGFLAGS) \
    -I$(SDKKEN)/include \
    -I$(SDKKEN)/src

SDK_LIBS = \
    $(SDKPROJ)/build/unix-user/$(SDK_MODEL_NAME)/libbcmsdk.a

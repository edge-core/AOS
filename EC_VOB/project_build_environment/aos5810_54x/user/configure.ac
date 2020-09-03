AC_INIT([accton_userlib], [0.1], [bug-report@address])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_FILES([Makefile
                 apps/Makefile
                 apps/backdoor/Makefile
                 apps/cli/Makefile
                 apps/snmpmgmt/v3/Makefile
                 apps/web/cgi/Makefile
                 core/Makefile
                 core/cmgr/Makefile
                 core/l2/Makefile
                 core/l2/bridge/Makefile
                 core/l2/bridge/extbrg/Makefile
                 core/l2/bridge/pri_mgmt/Makefile
                 core/l2/bridge/xstp/Makefile
                 core/l2/cn/Makefile
                 core/l2/dcbx/Makefile
                 core/l2/ets/Makefile
                 core/l2/l2mux/Makefile
                 core/l2/lldp/Makefile
                 core/l2/mlag/Makefile
                 core/l2/pfc/Makefile
                 core/l2/swctrl/Makefile
                 core/l2/swctrl/lacp/Makefile
                 core/l2/swctrl/rspan/Makefile
                 core/l2/vlan/Makefile
                 core/l2/pppoe_ia/Makefile
                 core/l3/Makefile
                 core/l3/iml/Makefile
                 core/l3/ipal/Makefile
                 core/l3/netcfg/Makefile
                 core/l3/amtrl3/Makefile
                 core/l3/ping/Makefile
                 core/l3/traceroute/Makefile
                 core/l3/vrrp/Makefile
                 core/l3/udphelper/Makefile
                 core/l3/vxlan/Makefile
                 core/mgmt/Makefile
                 core/mgmt/add/Makefile
                 core/mgmt/dbsync/txt/Makefile
                 core/mgmt/debug/Makefile
                 core/mgmt/dhcp/Makefile
                 core/mgmt/http/Makefile
                 core/mgmt/ledmgmt/Makefile
                 core/mgmt/mib2mgmt/Makefile
                 core/mgmt/ntp/Makefile
                 core/mgmt/sntp/Makefile
                 core/mgmt/stkmgmt/Makefile
                 core/mgmt/wtdmgmt/Makefile
                 core/mgmt/sys_callback/Makefile
                 core/mgmt/sysctrl/xor/Makefile
                 core/mgmt/syslog/Makefile
                 core/mgmt/sysmgmt/Makefile
                 core/mgmt/sysrsc/Makefile
                 core/mgmt/telnet/Makefile
                 core/mgmt/ucmgmt/Makefile
                 core/mgmt/xfer/Makefile
                 core/mgmt/cfgdb/Makefile
                 core/mgmt/dns/Makefile
                 core/mgmt/smtp/Makefile
                 core/mgmt/sflow/Makefile
                 core/qos_v2/Makefile
                 core/security/Makefile
                 core/security/dot1x/Makefile
                 core/security/keygen/Makefile
                 core/security/mgmt_ip_flt/Makefile
                 core/security/netaccess/Makefile
                 core/security/portauthsrvc/Makefile
                 core/security/portsec/Makefile
                 core/security/radius/Makefile
                 core/security/sshv2/Makefile
                 core/security/ssl/Makefile
                 core/security/tacacs/Makefile
                 core/security/userauth/Makefile
                 core/security/webauth/Makefile
                 core/security/appfilter/Makefile
                 driver/Makefile
                 driver/buffermgmt/Makefile
                 driver/devdrv/Makefile
                 driver/devdrv/onlp/Makefile
                 driver/devdrv/onlp/as5812_54x/Makefile
                 driver/devdrv/onlp/as5812_54t/Makefile
                 driver/devdrv/onlp/as6812_32x/Makefile
                 driver/fs/Makefile
                 driver/i2cdrv/Makefile
                 driver/iscagent/Makefile
                 driver/iscdrv/Makefile
                 driver/landrv/Makefile
                 driver/leddrv/Makefile
                 driver/phyaddr_access/Makefile
                 driver/swdrv/Makefile
                 driver/sysdrv/Makefile
                 driver/systime/Makefile
                 process/Makefile
                 process/auth_protocol_proc/Makefile
                 process/app_protocol_proc/Makefile
                 process/cli_proc/Makefile
                 process/core_util_proc/Makefile
                 process/driver_proc/Makefile
                 process/ip_service_proc/Makefile
                 process/l2_l4_proc/Makefile
                 process/netcfg_proc/Makefile
                 process/sflow_proc/Makefile
                 process/snmp_proc/Makefile
                 process/stkctrl_proc/Makefile
                 process/stktplg_proc/Makefile
                 process/sys_mgmt_proc/Makefile
                 process/sysinit_proc/Makefile
                 process/web_proc/Makefile
                 process/xfer_proc/Makefile
                 process/vrrp_proc/Makefile
                 process/sys_debug_proc/Makefile
                 process/sys_callback_proc/Makefile
                 process/watchdog_proc/Makefile
                 process/fpm_proc/Makefile
                 process/mac_sync_proc/Makefile
                 thirdpty/Makefile
                 thirdpty/snmp/net-snmp-5.1/Makefile
                 thirdpty/jansson/Makefile
                 thirdpty/inih/Makefile
               ])

#tool chain definition
#
AM_PROG_AR
AC_PROG_CC([${CROSS_COMPILE}gcc])
AC_SUBST([AR], [${CROSS_COMPILE}ar])
AC_PROG_LIBTOOL
AC_SUBST([LD], [${CROSS_COMPILE}ld])
AC_SUBST([STRIP], [${CROSS_COMPILE}strip])
AC_SUBST([CFLAGS], ['-Wall -Wno-unused-value -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-function -Wno-address -Wno-maybe-uninitialized -fno-strict-aliasing -O2 -g -rdynamic -DBROADCOM -D'${D_MODEL_NAME}])
AC_SUBST([LDFLAGS], ['-lrt -lm'])
AC_SUBST([ARFLAGS], ['cr'])

#Accton Source Directory
AC_ARG_VAR([ACCTONSRC_DIR], [the path of the top accton source directory])
AC_SUBST([ACCTONSRC_DIR], [${ACCSRC}])
AC_ARG_VAR([UBOOTSRC_DIR], [the path of the uboot source directory])
AC_SUBST([UBOOTSRC_DIR], ['$(top_builddir)/../prom/bootloader/uboot'])
AC_ARG_VAR([KERNELSRC_DIR], [the path of the Linux kernel source directory])
AC_SUBST([KERNELSRC_DIR], ['$(top_builddir)/../../../src/linux_kernel/linux-linux-3.2.65-1+deb7u2'])
AC_ARG_VAR([PROJECT_NAME], [project name for build environment])
AC_SUBST([PROJECT_NAME], [${PROJECT_NAME}])
AC_SUBST([BCM_CFGFLAGS], ['$(shell make -C ${SDKPROJ} show-cfgs|grep "CFGFLAGS"|sed -e "s/CFGFLAGS=//")'])

# Referenced by net-snmp
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([net/if.h])

# The patch is from "http://git.lrde.epita.fr/?p=spot.git;a=commitdiff;h=0e74b76521341f670f6b76f8ef24a6dcf6e3813b"
# Debian resets this to no, but this break both Spot and the libtool
# test suite itself.  Instead of requiring developer to install a
# non-patched version of Libtool on any Debian they use, we just
# cancel the effect of Debian's patch here.
link_all_deplibs=yes
link_all_deplibs_CXX=yes

AC_SUBST([WITHOUT_SDK_FLAG])
AM_CONDITIONAL([WITHOUT_SDK], [test "$WITHOUT_SDK_FLAG" == "1"])

AC_OUTPUT

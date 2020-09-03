SUBDIRS = l2_l4_proc ip_service_proc snmp_proc auth_protocol_proc \
	netcfg_proc stktplg_proc stkctrl_proc core_util_proc cli_proc \
	sys_mgmt_proc xfer_proc driver_proc sysinit_proc app_protocol_proc \
	web_proc sys_debug_proc \
	vrrp_proc sflow_proc sys_callback_proc watchdog_proc fpm_proc mac_sync_proc

sysetcdir = $(prefix)/../etc
install-data-hook:
	if [ ! -d $(sysetcdir)/init.d ]; then mkdir $(sysetcdir)/init.d; fi
	cp -f $(ACCTONSRC_DIR)/user/process/aos_process_fun $(sysetcdir)/init.d/
#       [ -f $(ACCPROJ)/utils/rc.accton.init.sed ] && \
#       sed -f $(ACCPROJ)/utils/rc.accton.init.sed $(ACCTONSRC_DIR)/user/process/rc.accton.init > $(sysetcdir)/rc.accton.init

ACCTON_LKM = acctonlkm
ACCTONSRC_DIR = $(ACCSRC)
KMODULE_OUTPUT = $(ACCROOTFS)/lib/modules/4.19.67
KBUILD_EXTRA_SYMBOLS := $(SDKPROJ)/build/linux-aos5810_54x/aos5810_54x/systems/bde/linux/kernel/kernel_module/Module.symvers

ACCTON_OBJS = \
	$(ACCTONSRC_DIR)/cmnlib/common/memmgmt/l_pt.c \
	$(ACCTONSRC_DIR)/cmnlib/kernel/kernel/linux/k_sysfun.c \
	$(ACCTONSRC_DIR)/cmnlib/kernel/kernel/linux/k_root.c \
	$(ACCTONSRC_DIR)/cmnlib/kernel/kernel/linux/k_umh.c \
	$(ACCTONSRC_DIR)/cmnlib/kernel/memmgmt/k_l_ipcmem.c \
	$(ACCTONSRC_DIR)/cmnlib/kernel/memmgmt/k_l_mm.c \
	$(ACCTONSRC_DIR)/kernel/core/l2/vlan/k_vlan_mgr.c \
	$(ACCTONSRC_DIR)/kernel/core/l3/iml/k_iml_mgr.c \
	$(ACCTONSRC_DIR)/kernel/core/l3/amtrl3/k_amtrl3_mgr.c \
	$(ACCTONSRC_DIR)/kernel/core/l3/netcfg/linux_porting/route_mgr/k_route_mgr.c \
	$(ACCTONSRC_DIR)/kernel/driver/vlan_net/vlan_net.c \
	$(ACCTONSRC_DIR)/kernel/driver/vlan_net/vlan_net_proc.c \
	$(ACCTONSRC_DIR)/kernel/driver/phyaddr_access/k_phyaddr_access.c \
	$(ACCTONSRC_DIR)/kernel/driver/phyaddr_access/${MODEL_NAME}/k_phyaddr_map.c \
	$(ACCTONSRC_DIR)/kernel/core/l3/ipal/k_ipal_if.c

include $(ACCTONSRC_DIR)/kernel/accton_mk_v4.19.67.include
EXTRA_CFLAGS += \
	-D${D_MODEL_NAME} \
	-DBROADCOM \
	-I$(ACCTONSRC_DIR)/cmnlib/kernel/kernel/linux \
	-I$(ACCTONSRC_DIR)/cmnlib/user/kernel/linux \
	-I$(ACCTONSRC_DIR)/cmnlib/user/memmgmt \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/kernel/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/kernel/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/kernel/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/kernel/driver/vlan_net/include \
	-I$(ACCTONSRC_DIR)/kernel/core/l3/ipal/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
        -I$(ACCTONSRC_DIR)/kernel/core/l3/netcfg/linux_porting/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/kernel/driver/phyaddr_access/${MODEL_NAME} \
        -I$(ACCTONSRC_DIR)/kernel/driver/phyaddr_access/include \
	-I$(ACCTONSRC_DIR)/user/driver/phyaddr_access \
        -I$(ACCTONSRC_DIR)/kernel/driver/i2c/include 

obj-m := $(ACCTON_LKM).o
$(ACCTON_LKM)-objs := $(notdir $(ACCTON_OBJS:.c=.o))

.PHONY: all install uninstall clean makelink

all: $(ACCTON_LKM).ko

$(ACCTON_LKM).ko: makelink $(notdir $(ACCTON_OBJS))
	$(MAKE) -C $(KERNDIR) SUBDIRS=$(PWD) O=$(KBUILD_OUTPUT) modules

install: $(ACCTON_LKM).ko
	mkdir -p $(KMODULE_OUTPUT)
	cp $(ACCTON_LKM).ko $(KMODULE_OUTPUT)

uninstall:
	rm $(KMODULE_OUTPUT)/$(ACCTON_LKM).ko

clean:
	rm -rf *.c *.o *.ko .*.cmd Module.symvers .tmp_versions 2> /dev/null

makelink:
	echo $(ACCTON_OBJS) | xargs -r -n1 ln -sf

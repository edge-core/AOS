#
# Makefile for the Linux VLAN layer.
#

#obj-$(CONFIG_VLAN_8021Q) += 8021q.o
#obj-$(CONFIG_VLAN_8021Q) += 8021q.o
#8021q-objs := vlan.o vlan_dev.o

#ifeq ($(CONFIG_PROC_FS),y)
#8021q-objs += vlanproc.o
#endif

O_TARGET := 8021q.o

mod-subdirs :=

export-objs := k_l_stdlib.o

subdir-y := 
subdir-m :=

obj-y := vlan.o vlan_dev.o vlanproc.o $(join $(subdir-y), $(patsubst %,/%.o,$(notdir $(subdir-y))))

include $(TOPDIR)/Rules.make

# include accton version definition file
include $(ACCPROJ)/versions

all-local: $(ACCSRC)/sysinclude/oem/$(PROJECT_NAME)/sys_ver.h

clean-local:
	rm -f $(ACCSRC)/sysinclude/oem/$(PROJECT_NAME)/sys_ver.h

$(ACCSRC)/sysinclude/oem/$(PROJECT_NAME)/sys_ver.h: $(ACCPROJ)/versions
	@echo "#ifndef SYS_VER_H" > $@; \
        echo "#define SYS_VER_H" >> $@; \
        echo "" >> $@; \
	echo "#define SYS_VER_POST_VER \"$(POST_VER)\"" >> $@; \
	echo "#define SYS_VER_ROOTFS_VER \"$(ROOTFS_VER)\"" >> $@; \
	echo "#define SYS_VER_POST_CUSTOMIZED_VER \"$(POST_CUSTOMIZED_VER)\"" >> $@; \
	echo "#define SYS_VER_ROOTFS_CUSTOMIZED_VER \"$(ROOTFS_CUSTOMIZED_VER)\"" >> $@; \
	echo "#define SYS_VER_LOADER_COMPAT_VER_ORG_PART_START \"$(LOADER_COMPAT_VER_ORG_PART_START)\"" >> $@; \
	echo "#define SYS_VER_LOADER_COMPAT_VER_ORG_PART_END \"$(LOADER_COMPAT_VER_ORG_PART_END)\"" >> $@; \
	echo "#define SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_START \"$(LOADER_COMPAT_VER_ACCTON_PART_START)\"" >> $@; \
	echo "#define SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_END \"$(LOADER_COMPAT_VER_ACCTON_PART_END)\"" >> $@; \
	echo "#define SYS_VER_KERNEL_COMPAT_VER_ORG_PART_START \"$(KERNEL_COMPAT_VER_ORG_PART_START)\"" >> $@; \
	echo "#define SYS_VER_KERNEL_COMPAT_VER_ORG_PART_END \"$(KERNEL_COMPAT_VER_ORG_PART_END)\"" >> $@; \
	echo "#define SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_START \"$(KERNEL_COMPAT_VER_ACCTON_PART_START)\"" >> $@; \
	echo "#define SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_END \"$(KERNEL_COMPAT_VER_ACCTON_PART_END)\"" >> $@; \
	echo "#endif" >> $@

SUBDIRS = . driver core apps thirdpty process

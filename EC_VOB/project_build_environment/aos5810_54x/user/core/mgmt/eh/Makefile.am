include $(top_builddir)/commdef.mk

all-local: $(ACCROOTFS)/usr/eh/eh_msg.xml

clean-local:
	rm -rf $(ACCPROJ)/eh_tmp \
	rm -rf $(ACCROOTFS)/usr/eh/eh_msg.xml

lib_LTLIBRARIES = libeh.la

libeh_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/eh/eh_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/eh/eh_mgr.c

libeh_la_CPPFLAGS = \
	-D_ISOC99_SOURCE \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/thirdpty/xml/libxml2-2.9.0/include

$(ACCROOTFS)/usr/eh/eh_msg.xml: $(ACCTONSRC_DIR)/user/core/mgmt/eh/include/eh_ui_msg_no.h
	$(ACCTONSRC_DIR)/user/core/mgmt/eh/gen_eh_msg_xml.sh

CFLAGS += -Werror

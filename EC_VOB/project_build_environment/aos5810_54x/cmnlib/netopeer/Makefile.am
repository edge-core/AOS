lib_LTLIBRARIES = libnetopeer.la

libnetopeer_la_SOURCES = \
	$(ACCTONSRC_DIR)/cmnlib/netopeer/datastore_util.c

libnetopeer_la_CPPFLAGS = \
	-DLIBC_USE_GNU \
	-DSYSCALL_DECLARATION_STYLE_2 \
	-I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/cmnlib/netopeer/include \
    -I${PROJECT_PATH}/user/thirdpty/xml/libxml2-2.9.0/include/libxml2
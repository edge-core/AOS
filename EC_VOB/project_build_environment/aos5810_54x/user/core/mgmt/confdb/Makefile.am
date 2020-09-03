# include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libconfdb.la

libconfdb_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/confdb/confdb.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/confdb/confdb_lock.c

libconfdb_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
    -g \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/confdb \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/confdb/include \
	-I${ACCTONSRC_DIR}/user/thirdpty/jansson/src \
	-I${ACCPROJ}/user/thirdpty/mongo-c-driver/src/mongoc \
	-I${ACCPROJ}/user/thirdpty/mongo-c-driver/src/libbson/src/bson

libconfdb_la_LIBADD = \
	$(COMMON_LIBS)
	${ACCPROJ}/user/thirdpty/mongo-c-driver/out/lib/libmongoc-1.0.a

CFLAGS += -Werror


# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/base-include
base_includes = src/base src

pkgconf_list+=libodatabase
LIBODATABASE_LIBS = -lodatabase
LIBODATABASE_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)/libodatabase $(PKGCONFIG_CPPFLAGS)
LIBODATABASE_ADD_PKGCONFIG := odata_provider_dir=$(pkglibexecdir)
odatabasedir=$(sdkdir)/libodatabase
odatabase_HEADERS = \
	src/common.h	\
	src/base/addr.h \
	src/base/alloc.h \
	src/base/array.h \
	src/base/auth.h \
	src/base/base64.h \
	src/base/buf.h \
	src/base/chars.h \
	src/base/cleanup.h \
	src/base/conffile.h \
	src/base/datetime.h \
	src/base/dump.h \
	src/base/env.h \
	src/base/err.h \
	src/base/file.h \
	src/base/find.h \
	src/base/format.h \
	src/base/getopt.h \
	src/base/html.h \
	src/base/http.h \
	src/base/httpstatuscodes.h \
	src/base/iprintf.h \
	src/base/log.h \
	src/base/pam.h \
	src/base/parse.h \
	src/base/path.h \
	src/base/pidfile.h \
	src/base/plugininfo.h \
	src/base/process.h \
	src/base/ptrarray.h \
	src/base/role.h \
	src/base/roundpow2.h \
	src/base/selector.h \
	src/base/shlib.h \
	src/base/sock.h \
	src/base/str.h \
	src/base/user.h

# this is a convenience library so we dont recompile
# base .c files repeatedly for each binary
lib_LTLIBRARIES += out/libodatabase.la
out_libodatabase_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(base_includes)) \
 	$(PAM_CFLAGS)

out_libodatabase_la_LDFLAGS = -ldl $(PAM_LIBS)

out_libodatabase_la_SOURCES = \
	src/base/addr.c \
	src/base/addrclient.c \
	src/base/alloc.c \
	src/base/allocprint.c \
	src/base/array.c \
	src/base/auth.c \
	src/base/authclient.c \
	src/base/authserver.c \
	src/base/base64dec.c \
	src/base/base64enc.c \
	src/base/buf.c \
	src/base/chars.c \
	src/base/cleanup.c \
	src/base/conffile.c \
	src/base/datetime.c \
	src/base/datetimetimestamp.c \
	src/base/dump.c \
	src/base/env.c \
	src/base/err.c \
	src/base/file.c \
	src/base/find.c \
	src/base/format.c \
	src/base/getopt.c \
	src/base/hex/hex.c \
	src/base/html.c \
	src/base/http.c \
	src/base/httpclient.c \
	src/base/httpdump.c \
	src/base/httpserver.c \
	src/base/httpstatuscodes.c \
	src/base/iprintf.c \
	src/base/log.c \
	src/base/pam.c \
	src/base/parse.c \
	src/base/path.c \
	src/base/pathdump.c \
	src/base/pathjoin.c \
	src/base/pathsplit.c \
	src/base/pidfile.c \
	src/base/plugininfo.c \
	src/base/process.c \
	src/base/ptrarray.c \
	src/base/role.c \
	src/base/roledump.c \
	src/base/roundpow2.c \
	src/base/selector.c \
	src/base/shlib.c \
	src/base/sock.c \
	src/base/socklisten.c \
	src/base/sockpeercreds.c \
	src/base/str.c \
	src/base/strbool.c \
	src/base/strhex.c \
	src/base/strint.c \
	src/base/strlcat.c \
	src/base/strlcpy.c	\
	src/base/user.c

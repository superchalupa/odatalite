# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/base-include
base_includes = src/base src

pkgconf_list+=libbase
LIBBASE_LIBS = -lbase
LIBBASE_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS)

# this is a convenience library so we dont recompile
# base .c files repeatedly for each binary
lib_LTLIBRARIES += out/libbase.la
out_libbase_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(base_includes)) \
 	$(PAM_CFLAGS)

out_libbase_la_LDFLAGS = -ldl $(PAM_LIBS)

out_libbase_la_SOURCES = \
	src/base/pidfile.c \
	src/base/parse.c \
	src/base/err.h \
	src/base/sock.h \
	src/base/cleanup.h \
	src/base/base64dec.c \
	src/base/path.c \
	src/base/selector.h \
	src/base/html.h \
	src/base/alloc.c \
	src/base/datetime.c \
	src/base/addrclient.c \
	src/base/role.h \
	src/base/httpstatuscodes.h \
	src/base/conffile.c \
	src/base/httpstatuscodes.c \
	src/base/datetime.h \
	src/base/httpserver.c \
	src/base/authclient.c \
	src/base/log.h \
	src/base/buf.c \
	src/base/plugininfo.h \
	src/base/env.c \
	src/base/cleanup.c \
	src/base/auth.h \
	src/base/roundpow2.c \
	src/base/ptrarray.h \
	src/base/hex/hex.c \
	src/base/file.c \
	src/base/strbool.c \
	src/base/chars.h \
	src/base/sock.c \
	src/base/find.c \
	src/base/shlib.h \
	src/base/pathdump.c \
	src/base/chars.c \
	src/base/html.c \
	src/base/httpdump.c \
	src/base/log.c \
	src/base/array.h \
	src/base/buf.h \
	src/base/process.c \
	src/base/base64enc.c \
	src/base/auth.c \
	src/base/process.h \
	src/base/conffile.h \
	src/base/format.h \
	src/base/parse.h \
	src/base/ptrarray.c \
	src/base/roundpow2.h \
	src/base/str.h \
	src/base/str.c \
	src/base/pam.h \
	src/base/addr.h \
	src/base/pam.c \
	src/base/iprintf.c \
	src/base/httpclient.c \
	src/base/array.c \
	src/base/plugininfo.c \
	src/base/http.h \
	src/base/env.h \
	src/base/sockpeercreds.c \
	src/base/path.h \
	src/base/strhex.c \
	src/base/addr.c \
	src/base/dump.c \
	src/base/socklisten.c \
	src/base/pidfile.h \
	src/base/pathjoin.c \
	src/base/getopt.h \
	src/base/base64.h \
	src/base/pathsplit.c \
	src/base/selector.c \
	src/base/allocprint.c \
	src/base/user.h \
	src/base/find.h \
	src/base/getopt.c \
	src/base/strlcat.c \
	src/base/alloc.h \
	src/base/dump.h \
	src/base/http.c \
	src/base/datetimetimestamp.c \
	src/base/authserver.c \
	src/base/err.c \
	src/base/strint.c \
	src/base/format.c \
	src/base/role.c \
	src/base/file.h \
	src/base/user.c \
	src/base/shlib.c \
	src/base/roledump.c \
	src/base/iprintf.h \
	src/base/strlcpy.c

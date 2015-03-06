# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include

BASELIB = out/libbase.la
client_includes = src

pkgconf_list+=libclient
LIBCLIENT_STATIC_LIBS = -lclient
LIBCLIENT_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# client .c files repeatedly for each binary
lib_LTLIBRARIES += out/libclient.la
out_libclient_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(client_includes))
out_libclient_la_LDFLAGS = \
	$(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER) \
	-shared -fPIC -Lout -lbase

out_libclient_la_SOURCES = \
	src/client/client.c \
	src/client/client.h

out_libclient_la_DEPENDENCIES=$(BASELIB)

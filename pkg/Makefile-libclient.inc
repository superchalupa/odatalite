# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

client_includes = src

lib_LTLIBRARIES += out/libclient.la
out_libclient_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(client_includes))
out_libclient_la_LIBADD = out/libbase.la
out_libclient_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER)

out_libclient_la_SOURCES = \
	src/client/client.c \
	src/client/client.h

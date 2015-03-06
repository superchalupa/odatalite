# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include
helloplugin_includes = src/plugins/helloplugin src/odata src

pkgconf_list+=libhelloplugin-static
LIBHELLOPLUGIN_STATIC_LIBS = -lhelloplugin
LIBHELLOPLUGIN_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# helloplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libhelloplugin.la
out_libhelloplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(helloplugin_includes))

out_libhelloplugin_la_LDFLAGS = -static

out_libhelloplugin_la_SOURCES = \
	src/plugins/hello/helloplugin.c


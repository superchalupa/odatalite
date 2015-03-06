# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include
pingplugin_includes = src/plugins/pingplugin src/odata src

pkgconf_list+=libpingplugin-static
LIBPINGPLUGIN_STATIC_LIBS = -lpingplugin
LIBPINGPLUGIN_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# pingplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libpingplugin.la
out_libpingplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(pingplugin_includes))

out_libpingplugin_la_LDFLAGS = -static

out_libpingplugin_la_SOURCES = \
	src/plugins/ping/pingplugin.c

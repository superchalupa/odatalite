# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include
echoplugin_includes = src/plugins/echoplugin src/odata src

pkgconf_list+=libechoplugin-static
LIBECHOPLUGIN_STATIC_LIBS = -lechoplugin
LIBECHOPLUGIN_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# echoplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libechoplugin.la
out_libechoplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(echoplugin_includes))

out_libechoplugin_la_LDFLAGS = -static

out_libechoplugin_la_SOURCES = \
	src/plugins/echo/echoplugin.c 

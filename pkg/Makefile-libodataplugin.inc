# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include
odataplugin_includes = src/plugins/odataplugin src/odata src

pkgconf_list+=libodataplugin-static
LIBODATAPLUGIN_STATIC_LIBS = -lodataplugin
LIBODATAPLUGIN_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# odataplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libodataplugin.la
out_libodataplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(odataplugin_includes))

out_libodataplugin_la_LDFLAGS = -static

out_libodataplugin_la_SOURCES = \
	src/plugins/odata/webpage.h \
	src/plugins/odata/scope.h \
	src/plugins/odata/providers.h \
	src/plugins/odata/providers.c \
	src/plugins/odata/scope.c \
	src/plugins/odata/odataplugin.h \
	src/plugins/odata/post.c \
	src/plugins/odata/options.h \
	src/plugins/odata/post.h \
	src/plugins/odata/webpage.c \
	src/plugins/odata/odataplugin.c

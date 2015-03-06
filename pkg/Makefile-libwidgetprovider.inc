# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

BASELIB = out/libbase.la
provider_includes = src

# add pkgconfig output files for widgetprovider. The following vars are used to construct the pkgconfig file
pkgconf_list+=libwidgetprovider
LIBWIDGETPROVIDER_LIBS=-lwidgetprovider
LIBWIDGETPROVIDER_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)

lib_LTLIBRARIES += out/libwidgetprovider.la
out_libwidgetprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER)
out_libwidgetprovider_la_LIBADD = 
out_libwidgetprovider_la_LDFLAGS = -shared -fPIC -Lout -lbase
out_libwidgetprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

# if the following two lines have *only* AM_* variable, then you can remove them and they will default to the AM_* value.
out_libwidgetprovider_la_CFLAGS = $(AM_CFLAGS)
out_libwidgetprovider_la_CXXFLAGS = $(AM_CXXFLAGS)

out_libwidgetprovider_la_SOURCES = \
	./src/odata/providers/widget/widgetprovider.c

out_libwidgetprovider_la_DEPENDENCIES=$(BASELIB)

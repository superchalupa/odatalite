# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_includes = src

# add pkgconfig output files for frogprovider. The following vars are used to construct the pkgconfig file
pkgconf_list+=libfrogprovider
LIBFROGPROVIDER_LIBS=-lfrogprovider
LIBFROGPROVIDER_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)

lib_LTLIBRARIES += out/libfrogprovider.la
out_libfrogprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,FROGPROVIDER)
out_libfrogprovider_la_LIBADD = 
out_libfrogprovider_la_LDFLAGS = -shared -fPIC
out_libfrogprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

# if the following two lines have *only* AM_* variable, then you can remove them and they will default to the AM_* value.
out_libfrogprovider_la_CFLAGS = $(AM_CFLAGS)
out_libfrogprovider_la_CXXFLAGS = $(AM_CXXFLAGS)

out_libfrogprovider_la_SOURCES = \
	./src/odata/providers/frog/FrogProvider.cpp \
	./src/odata/providers/frog/FrogProvider.h \
	./src/odata/providers/frog/strings.h


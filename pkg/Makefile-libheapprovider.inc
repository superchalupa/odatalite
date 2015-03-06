# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_includes = src

# add pkgconfig output files for heapprovider. The following vars are used to construct the pkgconfig file
pkgconf_list+=libheapprovider
LIBHEAPPROVIDER_LIBS=-lheapprovider
LIBHEAPPROVIDER_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)

lib_LTLIBRARIES += out/libheapprovider.la
out_libheapprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,HEAPPROVIDER)
out_libheapprovider_la_LIBADD = 
out_libheapprovider_la_LDFLAGS = -shared -fPIC
out_libheapprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

# if the following two lines have *only* AM_* variable, then you can remove them and they will default to the AM_* value.
out_libheapprovider_la_CFLAGS = $(AM_CFLAGS)
out_libheapprovider_la_CXXFLAGS = $(AM_CXXFLAGS)

out_libheapprovider_la_SOURCES = \
	./src/odata/providers/heap/HeapProvider.cpp \
	./src/odata/providers/heap/HeapProvider.h

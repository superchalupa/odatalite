# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_includes = src

# add pkgconfig output files for rootprovider. The following vars are used to construct the pkgconfig file
pkgconf_list+=librootprovider
LIBROOTPROVIDER_LIBS=-lrootprovider
LIBROOTPROVIDER_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)

lib_LTLIBRARIES += out/librootprovider.la
out_librootprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,ROOTPROVIDER)
out_librootprovider_la_LIBADD = 
out_librootprovider_la_LDFLAGS = -shared -fPIC
out_librootprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

# if the following two lines have *only* AM_* variable, then you can remove them and they will default to the AM_* value.
out_librootprovider_la_CFLAGS = $(AM_CFLAGS)
out_librootprovider_la_CXXFLAGS = $(AM_CXXFLAGS)

out_librootprovider_la_SOURCES = \
	./src/odata/providers/root/RootProvider.h \
	./src/odata/providers/root/RootProvider.cpp

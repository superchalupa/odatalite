# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

BASELIB = out/libbase.la
provider_includes = src

# add pkgconfig output files for metadataprovider. The following vars are used to construct the pkgconfig file
pkgconf_list+=libmetadataprovider
LIBMETADATAPROVIDER_LIBS=-lmetadataprovider
LIBMETADATAPROVIDER_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)

lib_LTLIBRARIES += out/libmetadataprovider.la
out_libmetadataprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,METADATAPROVIDER)
out_libmetadataprovider_la_LIBADD = 
out_libmetadataprovider_la_LDFLAGS = -shared -fPIC -Lout -lbase
out_libmetadataprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

# if the following two lines have *only* AM_* variable, then you can remove them and they will default to the AM_* value.
out_libmetadataprovider_la_CFLAGS = $(AM_CFLAGS)
out_libmetadataprovider_la_CXXFLAGS = $(AM_CXXFLAGS)

out_libmetadataprovider_la_SOURCES = \
	./src/odata/providers/metadata/metadataprovider.c \
	./src/odata/providers/metadata/ServiceRoot.c

out_libmetadataprovider_la_DEPENDENCIES=$(BASELIB)

#gen:
#	../../../csdl2c/csdl2c ../../../RedfishSchema/ServiceRoot.0.9.2.metadata




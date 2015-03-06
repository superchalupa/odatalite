# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

#EXTRA_DIST += src/odata-include
odata_includes = src/plugins/odata src

pkgconf_list+=libodata-static
LIBODATA_STATIC_LIBS = -lodata
LIBODATA_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS) 

# this is a convenience library so we dont recompile
# odata .c files repeatedly for each binary
lib_LTLIBRARIES += out/libodata.la
out_libodata_la_CPPFLAGS = \
	$(addprefix -I$(top_srcdir)/,$(odata_includes)) $(AM_CPPFLAGS) 
out_libodata_la_LDFLAGS = -static
out_libodata_la_SOURCES = \
	src/odata/setters.h \
	src/odata/jsonarray.h \
	src/odata/uriparser.h \
	src/odata/return.h \
	src/odata/pair.c \
	src/odata/jsonparser.c \
	src/odata/alloc.c \
	src/odata/providerentryprint.c \
	src/odata/providerentry.c \
	src/odata/deserialize.c \
	src/odata/queryoption.c \
	src/odata/serialize.h \
	src/odata/serialize.c \
	src/odata/odata.h \
	src/odata/compare.c \
	src/odata/types.c \
	src/odata/getters.c \
	src/odata/compare.h \
	src/odata/queryoption.h \
	src/odata/scratchbuf.h \
	src/odata/scope.h \
	src/odata/result.h \
	src/odata/uriparser.c \
	src/odata/chars.h \
	src/odata/setters.c \
	src/odata/chars.c \
	src/odata/uri.h \
	src/odata/name.h \
	src/odata/types.h \
	src/odata/jsonarray.c \
	src/odata/destructors.h \
	src/odata/getters.h \
	src/odata/destructors.c \
	src/odata/format.h \
	src/odata/odatamacros.h \
	src/odata/scope.c \
	src/odata/chars/chars.c \
	src/odata/odatacxx.h \
	src/odata/array.c \
	src/odata/result.c \
	src/odata/outstream.c \
	src/odata/csdl.h \
	src/odata/csdl.c \
	src/odata/jsonobject.h \
	src/odata/pair.h \
	src/odata/providers/root/RootProvider.h \
	src/odata/providers/root/RootProvider.cpp \
	src/odata/providers/widget/widgetprovider.c \
	src/odata/providers/frog/FrogProvider.h \
	src/odata/providers/frog/FrogProvider.cpp \
	src/odata/providers/frog/strings.h \
	src/odata/providers/metadata/metadataprovider.c \
	src/odata/providers/metadata/ServiceRoot.c \
	src/odata/providers/gadget/gadgetprovider.c \
	src/odata/providers/heap/HeapProvider.h \
	src/odata/providers/heap/HeapProvider.cpp \
	src/odata/providers/check/checkprovider.c \
	src/odata/outstream.h \
	src/odata/alloc.h \
	src/odata/object.c \
	src/odata/providerentry.h \
	src/odata/format.c \
	src/odata/jsonparser.h \
	src/odata/jsonobject.c \
	src/odata/odatacxxinlines.h \
	src/odata/json.h \
	src/odata/uri.c

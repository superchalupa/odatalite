# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

odata_includes = src/plugins/odata src

pkgconf_list+=libodata-static
LIBODATA_STATIC_LIBS = -lodata
LIBODATA_STATIC_CFLAGS=-I$(SDK_DEFAULT_INC_PATH) $(PKGCONFIG_CPPFLAGS)

# this is a convenience library so we dont recompile
# odata .c files repeatedly for each binary
lib_LTLIBRARIES += out/libodata.la
out_libodata_la_CPPFLAGS = $(addprefix -I$(top_srcdir)/,$(odata_includes)) $(AM_CPPFLAGS)
out_libodata_la_LIBADD = out/libbase.la
out_libodata_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,ODATA)
out_libodata_la_SOURCES = \
    src/odata/alloc.c \
    src/odata/alloc.h \
    src/odata/array.c \
    src/odata/chars.c \
    src/odata/chars.h \
    src/odata/compare.c \
    src/odata/compare.h \
    src/odata/deserialize.c \
    src/odata/destructors.c \
    src/odata/destructors.h \
    src/odata/format.c \
    src/odata/format.h \
    src/odata/getters.c \
    src/odata/getters.h \
    src/odata/jsonarray.c \
    src/odata/jsonarray.h \
    src/odata/json.h \
    src/odata/jsonobject.c \
    src/odata/jsonobject.h \
    src/odata/jsonparser.c \
    src/odata/jsonparser.h \
    src/odata/name.h \
    src/odata/object.c \
    src/odata/odatacxx.h \
    src/odata/odatacxxinlines.h \
    src/odata/odata.h \
    src/odata/odatamacros.h \
    src/odata/outstream.c \
    src/odata/outstream.h \
    src/odata/pair.c \
    src/odata/pair.h \
    src/odata/providerentry.c \
    src/odata/providerentry.h \
    src/odata/providerentryprint.c \
    src/odata/result.c \
    src/odata/result.h \
    src/odata/return.h \
    src/odata/scope.c \
    src/odata/scope.h \
    src/odata/scratchbuf.h \
    src/odata/serialize.c \
    src/odata/serialize.h \
    src/odata/setters.c \
    src/odata/setters.h \
    src/odata/types.c \
    src/odata/types.h \
    src/odata/uri.c \
    src/odata/uri.h \
    src/odata/uriparser.c \
    src/odata/uriparser.h \
    src/odata/queryoption.c \
    src/odata/queryoption.h \
    src/odata/csdl.h \
    src/odata/csdl.c

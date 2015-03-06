# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:


REQLIBS = \
	out/libodata.la \
	out/libbase.la
bin_includes = src src/odata

bin_PROGRAMS = out/json
out_json_DEPENDENCIES=$(REQLIBS)
out_json_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_json_LDADD = $(REQLIBS)
out_json_SOURCES = src/json/json.c

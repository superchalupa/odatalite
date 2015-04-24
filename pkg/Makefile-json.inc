# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

json_includes = src src/odata

bin_PROGRAMS += out/json
out_json_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(json_includes))
out_json_LDADD = out/libodata.la  out/libodatabase.la
out_json_SOURCES = src/json/json.c

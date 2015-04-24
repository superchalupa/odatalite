# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/olcli
out_olcli_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_olcli_LDADD = out/libodatabase.la  out/libodataclient.la
out_olcli_SOURCES = \
	src/olcli/olcli.c \
	src/olcli/options.h

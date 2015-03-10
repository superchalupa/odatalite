# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/phitcli
out_phitcli_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_phitcli_LDADD = out/libbase.la out/libclient.la
out_phitcli_SOURCES = \
	src/phitcli/phitcli.c

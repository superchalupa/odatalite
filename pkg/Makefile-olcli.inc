# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

REQLIBS = \
	out/libbase.la \
	out/libclient.la 

bin_includes = src

bin_PROGRAMS = out/olcli
out_olcli_DEPENDENCIES=$(REQLIBS)
out_olcli_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_olcli_LDADD = $(REQLIBS)
out_olcli_SOURCES = \
	src/olcli/olcli.c \
	src/olcli/options.h

# TODO
# .PHONY: check.sh

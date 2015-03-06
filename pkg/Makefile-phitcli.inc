# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

REQLIBS = \
	out/libbase.la \
	out/libclient.la 

bin_includes = src

bin_PROGRAMS = out/phitcli
out_phitcli_DEPENDENCIES=$(REQLIBS)
out_phitcli_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_phitcli_LDADD = $(REQLIBS)
out_phitcli_SOURCES = \
	src/phitcli/phitcli.c \
	src/phitcli/phitcli.h

# TODO
# .PHONY: check.sh   ### in original

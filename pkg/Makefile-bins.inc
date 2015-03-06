# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_includes = path/to/include

bin_PROGRAMS += out/binex

out_binex_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))

out_binex_LDADD = out/libexample.la 

out_binex_SOURCES = 
# put all source and headers here

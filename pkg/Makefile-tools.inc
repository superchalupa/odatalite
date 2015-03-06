# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/dlopen
out_dlopen_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_dlopen_LDADD = out/libbase.la
out_dlopen_SOURCES = src/tools/dlopen/dlopen.c

bin_PROGRAMS += out/gennstr
out_gennstr_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_gennstr_LDADD = out/libbase.la
out_gennstr_SOURCES = src/tools/gennstr/gennstr.c

bin_PROGRAMS += out/base64enc
out_base64enc_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_base64enc_LDADD = out/libbase.la
out_base64enc_SOURCES = src/tools/base64enc/base64enc.c

bin_PROGRAMS += out/base64dec
out_base64dec_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_base64dec_LDADD = out/libbase.la
out_base64dec_SOURCES = src/tools/base64dec/base64dec.c

bin_PROGRAMS += out/file2c
out_file2c_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_file2c_LDADD = 
out_file2c_SOURCES = src/tools/file2c/file2c.c

bin_PROGRAMS += out/octstr
out_octstr_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))
out_octstr_LDADD = out/libbase.la
out_octstr_SOURCES = src/tools/octstr/octstr.c

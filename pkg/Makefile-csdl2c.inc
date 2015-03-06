# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/csdl2c

out_csdl2c_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))	\
	$(EXPAT_CFLAGS)

out_csdl2c_LDADD = $(EXPAT_LIBS) out/libbase.la

out_csdl2c_SOURCES = \
	src/csdl2c/csdl2c.c \
	src/csdl2c/xml.c \
	src/csdl2c/xml.h

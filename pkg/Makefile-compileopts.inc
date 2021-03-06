# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

PKGCONFIG_CPPFLAGS = $(addprefix -D,$(PKGCONFIG_DEFINES))

# preprocessor flags only: include dirs (-I...) and defines (-Dxxx)
# dev can *add* flags using "make CPPFLAGS=..."
AM_CPPFLAGS = -D_GNU_SOURCE $(PKGCONFIG_CPPFLAGS)

# Linker flags
# dev can *add* flags using "make LDFLAGS=..."
AM_LDFLAGS = \
	-Wl,--no-undefined -Wl,--as-needed -Wl,--enable-new-dtags	\
	-Wl,--warn-common -Wl,--gc-sections

# C and C++ compiler flags. Use to set warnings, etc
AM_CFLAGS = -Wall -ffunction-sections -fdata-sections -pthread -fPIC  -std=gnu99 -Werror
AM_CXXFLAGS = -Wall -ffunction-sections -fdata-sections -pthread -fPIC -Werror

pcfiles = avsystem.pc avsysaudio.pc

all-local: $(pcfiles)

%.pc: %.pc
	cp $< $@

pkgconfigdir= $(libdir)/pkgconfig
pkgconfig_DATA= $(pcfiles)

CLEANFILES= $(pcfiles)

pcinfiles= avsystem.pc.in avsysaudio.pc.in
EXTRA_DIST= $(pcinfiles)


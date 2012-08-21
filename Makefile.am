if IS_SDK
SUBDIRS = pkgconfig-i386
else
SUBDIRS = pkgconfig-arm
endif
SUBDIRS += init

lib_LTLIBRARIES = libavsysaudio.la

includelibavsysaudiodir = $(includedir)/avsystem
includelibavsysaudio_HEADERS = include/avsystem.h \
				include/avsys-types.h \
				include/avsys-error.h \
				include/avsys-audio.h


libavsysaudio_la_SOURCES = avsys-common.c \
			 avsys-audio-logical-volume.c \
			 avsys-audio-handle.c \
			 avsys-audio-shm.c \
			 avsys-audio-sync.c \
			 avsys-audio-alsa.c \
			 avsys-audio.c \
			 avsys-audio-ascenario.c \
			 avsys-audio-path.c \
			 avsys-audio-pactrl.c

libavsysaudio_la_CFLAGS = $(ALSA_CFLAGS) -I$(srcdir)/include

libavsysaudio_la_LIBADD = $(ALSA_LIBS) -ldl -lrt -lpthread
libavsysaudio_la_LDFLAGS = -Wl,-init, __init_module
libavsysaudio_la_LDFLAGS += -Wl,-fini, __fini_module -version-info 0:1:0

libavsysaudio_la_SOURCES += avsys-audio-pasimple.c
libavsysaudio_la_CFLAGS += $(PASIMPLE_CFLAGS) $(PA_CFLAGS)
libavsysaudio_la_LIBADD += $(PASIMPLE_LIBS) $(PA_LIBS)

libavsysaudio_la_CFLAGS += $(ASCN_CFLAGS)
libavsysaudio_la_LIBADD += $(ASCN_LIBS)


if IS_SDK
libavsysaudio_la_CFLAGS += -DAUDIO_SDK_BUILD \
		           -D_MMFW_I386_ALL_SIMULATOR
endif			   

libavsysaudio_la_CFLAGS += $(MMLOG_CFLAGS) \
			-D__DEBUG_MODE__ \
			-D__USE_LOGMANAGER__ \
			-DMM_DEBUG_FLAG

libavsysaudio_la_LIBADD += $(MMLOG_LIBS) \
			   $(INIPARSER_LIBS)

bin_PROGRAMS = sound_initializer
sound_initializer_SOURCES = avsys-audio-initializer.c
sound_initializer_CFLAGS = -I$(srcdir)/include 

sound_initializer_CFLAGS += $(MMLOG_CFLAGS) 
sound_initializer_LDADD = libavsysaudio.la
sound_initializer_DEPENDENCIES = libavsysaudio.la

if WITH_AUDIOTEST
bin_PROGRAMS += avsys_audio_test
avsys_audio_test_SOURCES = audiotest/avsys-audio-test.c
avsys_audio_test_CFLAGS = -I$(srcdir)/include
avsys_audio_test_LDADD = libavsysaudio.la
avsys_audio_test_DEPENDENCIES = libavsysaudio.la

bin_PROGRAMS += avsys_volume_dump
avsys_volume_dump_SOURCES = audiotest/avsys-audio-volume-dump.c
avsys_volume_dump_LDADD = libavsysaudio.la
avsys_volume_dump_DEPENDENCIES = libavsysaudio.la
endif

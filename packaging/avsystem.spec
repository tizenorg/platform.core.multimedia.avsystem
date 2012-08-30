
Name:       avsystem
Summary:    Audio Video System
Version:    0.5.0
Release:    13
Group:      System/Libraries
License:    Apache-2.0
Source0:    avsystem-%{version}.tar.gz
BuildRequires: pkgconfig(alsa)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(mm-ta)
BuildRequires: pkgconfig(mm-log)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(libpulse)
BuildRequires: pkgconfig(libascenario)


%description
Audio Video System


%package devel
Summary:    Audio Video System Development headers and libraries
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Audio Video System Development headers and libraries.


%prep
%setup -q -n %{name}-%{version}


%build
%autogen
%configure \
%ifarch %{ix86}
	--enable-audiotest --enable-sdk
%else
	--enable-audiotest
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post 
/sbin/ldconfig
mkdir -p /etc/rc.d/rc3.d
mkdir -p /etc/rc.d/rc4.d
ln -sf /etc/rc.d/init.d/snd_init /etc/rc.d/rc3.d/S15snd_init
ln -sf /etc/rc.d/init.d/snd_init /etc/rc.d/rc4.d/S15snd_init

%postun 
/sbin/ldconfig

%files
%defattr(-,root,root,-)
/etc/rc.d/init.d/snd_init
/usr/bin/sound_initializer
/usr/bin/avsys_audio_test
/usr/bin/avsys_volume_dump
/usr/lib/libavsysaudio.so.0
/usr/lib/libavsysaudio.so.0.0.1

%files devel
/usr/lib/libavsysaudio.so
/usr/lib/pkgconfig/*.pc
/usr/include/avsystem/avsys-audio.h
/usr/include/avsystem/avsys-error.h
/usr/include/avsystem/avsys-types.h
/usr/include/avsystem/avsystem.h

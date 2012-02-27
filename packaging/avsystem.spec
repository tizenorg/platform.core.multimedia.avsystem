
Name:       avsystem
Summary:    Audio Video System
Version:    0.3.50
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    avsystem-%{version}.tar.bz2
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
	--enable-slp2 --enable-aquila --enable-pasimple 
%else
        --enable-slp2 --enable-sdk --enable-aquila --enable-pasimple
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install



%post -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
/etc/rc.d/init.d/snd_init
/usr/bin/camera_caps_generator
/usr/bin/sound_initializer
/usr/lib/libavsysaudio.so.0
/usr/lib/libavsysaudio.so.0.0.1
/usr/lib/libavsyscamera.so.0
/usr/lib/libavsyscamera.so.0.0.0

%files devel
/usr/lib/libavsysaudio.so
/usr/lib/pkgconfig/*.pc
/usr/lib/libavsyscamera.so
/usr/include/avsystem/avsys-audio.h
/usr/include/avsystem/avsys-cam-exif.h
/usr/include/avsystem/avsys-cam.h
/usr/include/avsystem/avsys-error.h
/usr/include/avsystem/avsys-types.h
/usr/include/avsystem/avsystem.h

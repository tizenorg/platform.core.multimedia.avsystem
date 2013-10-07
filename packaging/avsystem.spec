Name:       avsystem
Summary:    Audio Video System
Version:    0.5.5
Release:    0
Group:      System/Libraries
License:    Apache-2.0
Source0:    avsystem-%{version}.tar.gz
Source101:  packaging/avsystem.service
Source1001: 	avsystem.manifest

Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/systemctl
Requires(postun): /sbin/ldconfig
Requires(postun): /usr/bin/systemctl
Requires(preun): /usr/bin/systemctl

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


%package -n libavsysaudio
Summary:    Audio Video System libraries

%description -n libavsysaudio
Audio Video System libraries

%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .


%build
%autogen
%configure \
%if 0%{?simulator}
	--enable-audiotest --enable-sdk
%else
	--enable-audiotest
%endif

make %{?jobs:-j%jobs}

%install
%make_install

mkdir -m 755 -p %{buildroot}/%{_sysconfdir}/rc.d/rc3.d/
ln -s ../init.d/snd_init %{buildroot}/%{_sysconfdir}/rc.d/rc3.d/S15snd_init
mkdir -m 755 -p %{buildroot}/%{_sysconfdir}/rc.d/rc4.d/
ln -s ../init.d/snd_init %{buildroot}/%{_sysconfdir}/rc.d/rc4.d/S15snd_init

mkdir -m 755 -p %{buildroot}/usr/lib/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE101 %{buildroot}/usr/lib/systemd/system/avsystem.service
ln -s ../avsystem.service %{buildroot}/usr/lib/systemd/system/multi-user.target.wants/avsystem.service

%preun
if [ $1 == 0 ]; then
    systemctl stop avsystem.service
fi

%post
systemctl daemon-reload
if [ $1 == 1 ]; then
    systemctl restart avsystem.service
fi

%post -n libavsysaudio
/sbin/ldconfig

%postun
systemctl daemon-reload

%postun -n libavsysaudio
/sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/rc.d/init.d/snd_init
%{_sysconfdir}/rc.d/rc3.d/S15snd_init
%{_sysconfdir}/rc.d/rc4.d/S15snd_init
%{_bindir}/*
/usr/lib/systemd/system/avsystem.service
/usr/lib/systemd/system/multi-user.target.wants/avsystem.service

%files devel
%manifest %{name}.manifest
%{_libdir}/pkgconfig/*.pc
%{_libdir}/*.so
%{_includedir}/avsystem/*.h

%files -n libavsysaudio
%manifest %{name}.manifest
%{_libdir}/lib*.so.*

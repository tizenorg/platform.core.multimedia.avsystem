Name:       avsystem
Summary:    Audio Video System
Version:    0.5.5
Release:    0
Group:      System/Libraries
License:    Apache-2.0
Source0:    avsystem-%{version}.tar.gz
Source101:  packaging/avsystem.service
Source1001: avsystem.manifest

Requires(post):   /sbin/ldconfig
Requires(post):   /usr/bin/systemctl
Requires(postun): /sbin/ldconfig
Requires(postun): /usr/bin/systemctl
Requires(preun):  /usr/bin/systemctl

BuildRequires: pkgconfig
BuildRequires: pkgconfig(alsa)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(mm-ta)
BuildRequires: pkgconfig(mm-log)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(libpulse)
BuildRequires: pkgconfig(libascenario)

%description
Audio Video System package.


%package devel
Summary:    Audio Video System (dev)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Audio Video System development headers and libraries.


%package -n libavsysaudio
Summary:    Audio Video System (libs)

%description -n libavsysaudio
Audio Video System libraries.


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

%__make %{?_smp_mflags}


%install
%make_install

mkdir -m 755 -p %{buildroot}%{_unitdir}/multi-user.target.wants
install -m 0644 %SOURCE101 %{buildroot}%{_unitdir}/avsystem.service
ln -sf ../avsystem.service %{buildroot}%{_unitdir}/multi-user.target.wants/avsystem.service


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
%{_bindir}/*
%{_unitdir}/avsystem.service
%{_unitdir}/multi-user.target.wants/avsystem.service

%files devel
%manifest %{name}.manifest
%{_libdir}/pkgconfig/*.pc
%{_libdir}/*.so
%{_includedir}/avsystem/*.h

%files -n libavsysaudio
%manifest %{name}.manifest
%{_libdir}/lib*.so.*

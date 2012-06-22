Name:       avsystem
Summary:    Audio Video System
Version:    0.4.13
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    avsystem-%{version}.tar.gz
Source101:  packaging/avsystem.service
Source1001: packaging/avsystem.manifest

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


%prep
%setup -q -n %{name}-%{version}


%build
cp %{SOURCE1001} .
%autogen
%configure \
%ifarch %{ix86}
	--enable-slp2 --enable-aquila --enable-pasimple 
%else
        --enable-slp2 --enable-sdk --enable-aquila --enable-pasimple
%endif

make %{?jobs:-j%jobs}

%install
%make_install


mkdir -p %{buildroot}/%{_sysconfdir}/rc.d/rc3.d/
ln -s ../init.d/snd_init %{buildroot}/%{_sysconfdir}/rc.d/rc3.d/S30snd_init
mkdir -p %{buildroot}/%{_sysconfdir}/rc.d/rc4.d/
ln -s ../init.d/snd_init %{buildroot}/%{_sysconfdir}/rc.d/rc4.d/S30snd_init

mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE101 %{buildroot}%{_libdir}/systemd/system/avsystem.service
ln -s ../avsystem.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/avsystem.service


%preun
if [ $1 == 0 ]; then
    systemctl stop avsystem.service
fi

%post
/sbin/ldconfig
systemctl daemon-reload
if [ $1 == 1 ]; then
    systemctl restart avsystem.service
fi

%postun
/sbin/ldconfig
systemctl daemon-reload

%files
%manifest avsystem.manifest
%{_sysconfdir}/rc.d/init.d/snd_init
%{_sysconfdir}/rc.d/rc3.d/S30snd_init
%{_sysconfdir}/rc.d/rc4.d/S30snd_init
%{_bindir}/*
%{_libdir}/lib*.so.*
%{_libdir}/systemd/system/avsystem.service
%{_libdir}/systemd/system/multi-user.target.wants/avsystem.service

%files devel
%manifest avsystem.manifest
%{_libdir}/pkgconfig/*.pc
%{_libdir}/*.so
/usr/include/avsystem/*.h


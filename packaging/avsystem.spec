Name:       avsystem
Summary:    Audio Video System
Version:    0.4.13
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    avsystem-%{version}.tar.gz
Source1001: packaging/avsystem.manifest 
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


%post 
/sbin/ldconfig
mkdir -p /etc/rc.d/rc3.d/
ln -s ../init.d/snd_init /etc/rc.d/rc3.d/S30snd_init
mkdir -p /etc/rc.d/rc4.d/
ln -s ../init.d/snd_init /etc/rc.d/rc4.d/S30snd_init

%files
%manifest avsystem.manifest
/etc/rc.d/init.d/snd_init
/usr/bin/*
/usr/lib/lib*.so.*

%files devel
%manifest avsystem.manifest
/usr/lib/pkgconfig/*.pc
/usr/lib/*.so
/usr/include/avsystem/*.h


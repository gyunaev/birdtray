Name:         birdtray
Version:      1.7.0
Release:      1%{?dist}
Epoch:        0
License:      GPLv3
Group:        System Environment/Shells
Source0:      https://github.com/gyunaev/%{name}/archive/RELEASE_%{version}.tar.gz
URL:          https://github.com/gyunaev/birdtray
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: cmake3 gcc-c++ desktop-file-utils
BuildRequires: sqlite-devel qt5-qtbase-devel qt5-qtx11extras-devel
%if 0%{?fedora} || 0%{?rhel} > 7
Recommends:    qt5-qttranslations
%endif


Summary: Birdtray is a free system tray notification for new mail for Thunderbird

%description
Birdtray is a system tray new mail notification for Thunderbird, which does not require extensions.

%prep
%setup -q -n %{name}-RELEASE_%{version}

%build
mkdir %{_target_platform}
pushd %{_target_platform}
%cmake3 -DCMAKE_BUILD_TYPE=Release \
        -DOPT_THUNDERBIRD_CMDLINE=%{_bindir}/thunderbird \
        ..
make %{?_smp_mflags}
popd

%install
make install DESTDIR=%{buildroot} -C %{_target_platform}

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :


%files
%{_bindir}/birdtray
%{_datadir}/applications/com.ulduzsoft.Birdtray.desktop
%{_datadir}/icons/*
%{_datadir}/metainfo/com.ulduzsoft.Birdtray.appdata.xml
%{_datadir}/ulduzsoft/%{name}/translations/*


%changelog
* Wed Feb 26 2020 Raven <raven@sysadmins.ws> - 1.7.0-1
- initial build

%undefine __cmake_in_source_build

Name:           wifiheatmap
Version:        0.1
Release:        0.1.alpha3%{?dist}
Summary:        WiFi heat map survey tool

License:        GPLv2
URL:            https://github.com/weimens/wifiheatmap
Source0:        wifiheatmap-0.1-alpha.3.tar.gz

BuildRequires:  gcc-c++ cmake make
BuildRequires:  qt5-qtbase-devel qt5-qtdeclarative-devel libnl3-devel quazip-qt5-devel iperf3-devel
BuildRequires:  desktop-file-utils libappstream-glib
BuildRequires:  git
Requires:       qt5-qtquickcontrols2
Requires:       %{_sbindir}/iw
Requires:       %{_bindir}/pkexec

%description
WiFi heat map survey tool.


%prep
%autosetup  -n wifiheatmap-0.1-alpha


%build
%cmake -DTRIGGER_SCAN_BIN=%{_bindir}/%{name}_trigger_scan
%cmake_build

%install
pushd %{_target_platform}
%make_install
popd

%files
%{_bindir}/%{name}
%{_bindir}/%{name}_trigger_scan
%{_metainfodir}/*.appdata.xml
%{_datadir}/applications/*.desktop
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%{_datadir}/mime/packages/*

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/com.github.weimens.wifiheatmap.desktop
%{?fedora:appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.appdata.xml}

%post
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%changelog
* Sun Mar 21 2020 Clemens Weissbacher
- 
* Fri Jun 19 2020 Clemens Weissbacher
- 
* Sat Feb 22 2020 Clemens Weissbacher
- 

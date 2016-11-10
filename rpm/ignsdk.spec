Summary: IGOS Nusantara Software Development Kit (IGNSDK) - launcher
Name: ignsdk
Version: 1.1.7
Release: 1%{?dist}
License: BSD
Group: Development/Other
URL: http://igos-nusantara.or.id
Source0: https://github.com/ubunteroz/ignsdk-debian/archive/%{version}/ignsdk-debian-%{version}.tar.gz
BuildRequires: qt5-qtwebkit-devel
BuildRequires: gcc-c++
BuildRequires: qt5-qtdeclarative-devel
BuildRequires: qt5-qtserialport-devel
Requires: qt5-qtwebkit
Requires: qt5-qtbase-mysql
Requires: qt5-qtdeclarative
Requires: qt5-qtxmlpatterns
Requires: qt5-qtserialport

%description
IGOS Nusantara Software Development Kit (IGNSDK) - launcher

%package devtools
Summary: IGOS Nusantara Software Development Kit (IGNSDK) - development tools
Group: Development/Other
Requires: ignsdk
Requires: rpm-build

%description devtools
IGOS Nusantara Software Development Kit (IGNSDK) - development tools

%prep
%setup -q -n %{name}-qt-master

%install
install -d -m 755 $RPM_BUILD_ROOT/usr/bin
install -d -m 755 $RPM_BUILD_ROOT/etc
install -d -m 755 $RPM_BUILD_ROOT/usr/share/ign-sdk/examples
install -d -m 755 $RPM_BUILD_ROOT/usr/share/ign-sdk/template
install -d -m 755 $RPM_BUILD_ROOT/usr/share/ign-sdk/bin
install -d -m 755 $RPM_BUILD_ROOT/usr/share/man/man1
qmake-qt5
make
cp -rf bin/ignsdk $RPM_BUILD_ROOT/usr/bin
cp -rf scripts/ignsdk-app-creator $RPM_BUILD_ROOT/usr/bin
cp -rf scripts/ignsdk-ign-creator $RPM_BUILD_ROOT/usr/share/ign-sdk/bin
cp -rf scripts/ignsdk-app-builder $RPM_BUILD_ROOT/usr/bin
cp -rf scripts/ignsdk-ign-builder $RPM_BUILD_ROOT/usr/share/ign-sdk/bin
gzip -c docs/ignsdk.1 > $RPM_BUILD_ROOT/%{_mandir}/man1/ignsdk.1.gz
gzip -c docs/ignsdk-app-builder.1 > $RPM_BUILD_ROOT/%{_mandir}/man1/ignsdk-app-builder.1.gz
gzip -c docs/ignsdk-app-creator.1 > $RPM_BUILD_ROOT/%{_mandir}/man1/ignsdk-app-creator.1.gz
echo "ign" > $RPM_BUILD_ROOT/etc/ignsdk-dist
cp -rf examples/* $RPM_BUILD_ROOT/usr/share/ign-sdk/examples
cp -rf template/main.tpl $RPM_BUILD_ROOT/usr/share/ign-sdk/template
cp -rf template/app.spec $RPM_BUILD_ROOT/usr/share/ign-sdk/template

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/usr/bin/ignsdk
/usr/share/ign-sdk/*
%doc LICENSE.BSD third-party.txt README.md
%doc %{_mandir}/man1/ignsdk.1.gz

%files devtools
%defattr(-,root,root,-)
/usr/bin/ignsdk-app-creator
/usr/bin/ignsdk-app-builder
/usr/share/ign-sdk/bin/ignsdk-ign-creator
/usr/share/ign-sdk/bin/ignsdk-ign-builder
/usr/share/ign-sdk/template/main.tpl
/usr/share/ign-sdk/template/app.spec
/etc/ignsdk-dist
%doc %{_mandir}/man1/ignsdk-app-builder.1.gz
%doc %{_mandir}/man1/ignsdk-app-creator.1.gz

%changelog
* Wed Mar 11 2015 ibnu yahya <anak10thn@gmail.com>
- UPDATE : version 1.1.7
- NEW API : SQL
- NEW API : SERIAL PORT

* Tue Feb 24 2015 ibnu yahya <anak10thn@gmail.com>
- UPDATE : version 1.1.7 alpha
- NEW API : print
- NEW API : json parser

* Wed Oct 15 2014 ibnu yahya <linux@toroo.org>
- UPDATE : version 1.1.6
- NEW API : Include Javascript Evaluate #138
- NEW API : Save File Dialog #137
- FIX BUGS : Read File #136
- FIX BUGS : Exec Function #133
- FIX BUGS : setUrl #132

* Mon May 12 2014 ibnu yahya <linux@toroo.org>
- FIX : live preview
- UDPATE : final version 1.1.5

* Mon Apr 07 2014 ibnu yahya <linux@toroo.org>
- ADD: proxy API

* Thu Mar 27 2014 ibnu yahya <linux@toroo.org>
- UPDATE: ignsdk alpha 1.1.5
- FIX BUGS : Open dir dialog OSX (fs API)
- FIX BUGS : error handling query SQL (sql API)
- CHANGE : convert output Query SQL API to JSON (sql API)
- ADD API : Info (filesystem api)
- ADD API : list (filesystem api)
- FIX : list all file watcher to live preview
- ADD CLASS : network
- ADD Function : Live preview
- ADD API : Check Your IP (network api)
- FIX : API name

* Wed Mar 19 2014 ibnu yahya <linux@toroo.org>
- FIX: bugs global settings
- ADD: debugging parameter

* Tue Mar 18 2014 ibnu yahya <linux@toroo.org>
- FIX: bugs template
- FIX: bugs ignsdk creator and builder

* Mon Mar 17 2014 ibnu yahya <linux@toroo.org>
- ADD: proxy API
- Version 1.1.4
- FIX: bugs ignsdk-app-builder

* Sun Dec 29 2013 ibnu yahya <linux@toroo.org>
- ADD: new filesystem API
- FIX: MAN page

* Sun Dec 08 2013 ibnu yahya <linux@toroo.org>
- UPDATE version to 1.1.3
- UPDATE ignsdk app creator and builder

* Thu Nov 28 2013 ibnu yahya <linux@toroo.org>
- UPDATE final version
- ADD man page app creator and builder
- ADD doc ignsdk

* Thu Nov 21 2013 ibnu yahya <linux@toroo.org>
- REMOVE Bootstrap

* Mon Nov 11 2013 ibnu yahya <linux@toroo.org>
- ADD Manual Page

* Sun Nov 10 2013 ibnu yahya <linux@toroo.org>
- move cliOut(), exec(), hash() to sys() init
- Open file dialog if project directory option is null
- update filesystem() ini API
- FIX trigger action
- ADD trigger action copy,cut,paste,redo,undo

* Sun Oct 13 2013 ibnu yahya <linux@toroo.org>
- Build nightly
- Fix spec file dependency

* Sat Oct 12 2013 ibnu yahya <linux@toroo.org>
- Porting ignsdk 1.1.1 to Qt 5

* Wed Sep 25 2013 ibnu yahya <linux@toroo.org>
- Add SQL API stable 1
- Release stable minor nightly

* Wed Sep 11 2013 ibnu yahya <linux@toroo.org>
- Add ignsdk-dist
- Add test file
- Add devtools package
- Add IGNSDK Builder script

* Tue Sep 10 2013 ibnu yahya <linux@toroo.org>
- Update version to 1.0.9
- Add ignsdk project builder

* Tue Jul 30 2013 ibnu yahya <linux@toroo.org>
- ADD dir API
- Update File API

* Mon Jul 29 2013 ibnu yahya <linux@toroo.org>
- ADD template

* Tue Jul 23 2013 ibnu yahya <linux@toroo.org>
- FIX Api
- change version build b2

* Mon Feb 18 2013 ibnu yahya <linux@toroo.org>
- First Build

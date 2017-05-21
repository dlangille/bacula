%global langs en pl
%global destdir build
%global metaname baculum

Summary:	API layer to Baculum WebGUI tool for Bacula Community program
Name:		baculum
Version:	9.0.0
Release:	1%{?dist}
License:	AGPLv3
Group:		Applications/Internet
URL:		http://bacula.org/
Source0:	bacula-gui-9.0.0.tar.gz
BuildRequires:	systemd-units
BuildRequires:	checkpolicy
Requires:	bacula-console
# Lower version of PHP ( < 5.3.4) does not provide php-mysqlnd db driver
# and from this reason the lowest is 5.3.4
Requires:	php >= 5.3.4
Requires:	php-bcmath
Requires:	php-common
Requires:	php-mbstring
Requires:	php-mysqlnd
Requires:	php-pdo
Requires:	php-pgsql
Requires:	php-xml
BuildArch:	noarch

%description
The Baculum program allows the user to administer and manage Bacula jobs.
By using Baculum it is possible to execute backup/restore operations, monitor
current Bacula jobs, configure Bacula, media management and others. Baculum has
integrated web console that communicates with Bacula bconsole program.

%package common
Summary:		Common libraries for Baculum
Group:			Applications/Internet

%description common
This package provides the common libraries for Baculum.
This module is a part of Baculum.

%package api
Summary:		Baculum API files
Requires:		%name-common = %version-%release
Group:			Applications/Internet
Requires:		bacula-console
# Lower version of PHP ( < 5.3.4) does not provide php-mysqlnd db driver
# and from this reason the lowest is 5.3.4
Requires:		php >= 5.3.4
Requires:		php-bcmath
Requires:		php-common
Requires:		php-mysqlnd
Requires:		php-pdo
Requires:		php-pgsql
Requires:		php-xml

%description api
This package provides the API files for Baculum.
This module is a part of Baculum.

%package web
Summary:		Baculum API files
Requires:		%name-common = %version-%release
Group:			Applications/Internet
# Lower version of PHP ( < 5.3.4) does not provide php-mysqlnd db driver
# and from this reason the lowest is 5.3.4
Requires:		php >= 5.3.4
Requires:		php-common
Requires:		php-mbstring
Requires:		php-xml

%description web
This package provides the Web files for Baculum.
This module is a part of Baculum.

%package api-httpd
Summary:		Apache configuration for Baculum API
Requires:		%name-api = %version-%release
Group:			Applications/Internet
Requires:		httpd
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-api-lighttpd, %{name}-web-lighttpd

%description api-httpd
This package provides the Apache configuration for Baculum API.
By using this module it is possible to run Baculum API in Apache environment.

%package api-lighttpd
Summary:		Lighttpd configuration for Baculum API
Requires:		%name-api = %version-%release
Group:			Applications/Internet
Requires:		lighttpd
Requires:		lighttpd-fastcgi
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-api-httpd, %{name}-web-httpd

%description api-lighttpd
This package provides the Lighttpd configuration for Baculum API.
By using this module it is possible to run Baculum API in Lighttpd environment.

%package web-httpd
Summary:		Apache configuration for Baculum WebGUI
Requires:		%name-web = %version-%release
Group:			Applications/Internet
Requires:		httpd
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-web-lighttpd, %{name}-api-lighttpd

%description web-httpd
This package provides the Apache configuration for Baculum WebGUI.
By using this module it is possible to run Baculum WebGUI in Apache environment.

%package web-lighttpd
Summary:		Lighttpd configuration for Baculum WebGUI
Requires:		%name-web = %version-%release
Group:			Applications/Internet
Requires:		lighttpd
Requires:		lighttpd-fastcgi
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-web-httpd, %{name}-api-httpd

%description web-lighttpd
This package provides the Lighttpd configuration for Baculum WebGUI.
By using this module it is possible to run Baculum WebGUI in Lighttpd environment.

%prep
%setup -n bacula-gui-%version/baculum

%build
# Execute files preparation in build directory by Makefile
make build DESTDIR=%{destdir}
# Remove these cache directories, because here will be symbolic links
rmdir %{destdir}/%{_datadir}/%{metaname}/htdocs/assets
rmdir %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/runtime
for lang in  %{langs}; do
	rm %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
	rm %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
done

%install
cp -ra build/. %{buildroot}
%find_lang %{metaname} --all-name

%post common
# these symbolic links indicates to Baculum's cache directory
ln -s  %{_localstatedir}/cache/%{metaname} %{_datadir}/%{metaname}/htdocs/assets
ln -s  %{_localstatedir}/cache/%{metaname} %{_datadir}/%{metaname}/htdocs/protected/runtime

%post api
# because framework does not use system locale dir, here are linked
# locale files to framework location
for lang in  %{langs}; do
	ln -s  %{_datadir}/locale/${lang}/LC_MESSAGES/%{metaname}-api.mo \
		%{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
done

%post web
# because framework does not use system locale dir, here are linked
# locale files to framework location
for lang in  %{langs}; do
	ln -s  %{_datadir}/locale/${lang}/LC_MESSAGES/%{metaname}-web.mo \
		%{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
done

%post api-httpd
%systemd_post httpd.service
ln -s  %{_sysconfdir}/%{metaname}/Config-api-apache %{_datadir}/%{metaname}/htdocs/protected/API/Config

%post api-lighttpd
%systemd_post baculum-api-lighttpd.service
ln -s  %{_sysconfdir}/%{metaname}/Config-api-lighttpd %{_datadir}/%{metaname}/htdocs/protected/API/Config

%post web-httpd
%systemd_post httpd.service
ln -s  %{_sysconfdir}/%{metaname}/Config-web-apache %{_datadir}/%{metaname}/htdocs/protected/Web/Config

%post web-lighttpd
%systemd_post baculum-web-lighttpd.service
ln -s  %{_sysconfdir}/%{metaname}/Config-web-lighttpd %{_datadir}/%{metaname}/htdocs/protected/Web/Config

%preun common
rm %{_datadir}/%{metaname}/htdocs/assets
rm %{_datadir}/%{metaname}/htdocs/protected/runtime

%preun api
for lang in  %{langs}; do
	rm %{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
done

%preun web
for lang in  %{langs}; do
	rm %{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
done

%preun api-httpd
%systemd_preun httpd.service
if [ $1 -lt 1 ] ; then
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf \
		%{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf.apache
	# remove debug files if any
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log ] ||
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api*.log
	rm %{_datadir}/%{metaname}/htdocs/protected/API/Config
fi

%preun web-httpd
%systemd_preun httpd.service
if [ $1 -lt 1 ] ; then
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf \
		%{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf.apache
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf \
		%{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf.apache
	# remove debug files if any
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log ] ||
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web*.log
	rm %{_datadir}/%{metaname}/htdocs/protected/Web/Config
fi

%preun api-lighttpd
%systemd_preun baculum-api-lighttpd.service
if [ $1 -lt 1 ] ; then
	# stop api service
	/sbin/service baculum-api-lighttpd stop &>/dev/null || :
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf \
		%{_datadir}/%{metaname}/htdocs/protected/API/Config/api.conf.lighttpd
	# remove debug files if any
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log ] ||
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api*.log
	rm %{_datadir}/%{metaname}/htdocs/protected/API/Config
fi

%preun web-lighttpd
%systemd_preun baculum-web-lighttpd.service
if [ $1 -lt 1 ] ; then
	# stop web service
	/sbin/service baculum-web-lighttpd stop &>/dev/null || :
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf \
		%{_datadir}/%{metaname}/htdocs/protected/Web/Config/settings.conf.lighttpd
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf ] ||
		mv %{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf \
		%{_datadir}/%{metaname}/htdocs/protected/Web/Config/hosts.conf.lighttpd
	# remove debug files if any
	[ ! -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log ] ||
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web*.log
	rm %{_datadir}/%{metaname}/htdocs/protected/Web/Config
fi

%postun api-httpd
%systemd_postun_with_restart httpd.service

%postun web-httpd
%systemd_postun_with_restart httpd.service

%postun api-lighttpd
%systemd_postun_with_restart baculum-api-lighttpd.service

%postun web-lighttpd
%systemd_postun_with_restart baculum-web-lighttpd.service

%files -f %{metaname}.lang common
%defattr(-,root,root)
%{_datadir}/%{metaname}/htdocs/protected/Common
%{_datadir}/%{metaname}/htdocs/protected/application.xml
%{_datadir}/%{metaname}/htdocs/framework
%{_datadir}/%{metaname}/htdocs/themes
%{_datadir}/%{metaname}/htdocs/LICENSE
%{_datadir}/%{metaname}/htdocs/AUTHORS
%{_datadir}/%{metaname}/htdocs/README
%{_datadir}/%{metaname}/htdocs/INSTALL
%{_datadir}/%{metaname}/htdocs/index.php
%license LICENSE
%doc AUTHORS README


%files api
%defattr(-,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{metaname}/htdocs/protected/API/Config
%exclude %{_datadir}/%{metaname}/htdocs/protected/API/Logs
%{_datadir}/%{metaname}/htdocs/protected/API

%files web
%defattr(-,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{metaname}/htdocs/protected/Web/Config
%exclude %{_datadir}/%{metaname}/htdocs/protected/Web/Logs
%{_datadir}/%{metaname}/htdocs/protected/Web


%files api-httpd
%defattr(644,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{metaname}/htdocs/protected/API/Config
# Apache logs are stored in /var/log/httpd/
%config(noreplace) %{_sysconfdir}/httpd/conf.d/%{metaname}-api.conf
%attr(755,apache,apache) %{_localstatedir}/cache/%{metaname}/
%attr(700,apache,apache) %{_sysconfdir}/%{metaname}/Config-api-apache/
%attr(600,apache,apache) %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users
%attr(755,apache,apache) %{_datadir}/%{metaname}/htdocs/protected/API/Logs

%files web-httpd
%defattr(644,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{metaname}/htdocs/protected/Web/Config
# Apache logs are stored in /var/log/httpd/
%config(noreplace) %{_sysconfdir}/httpd/conf.d/%{metaname}-web.conf
%attr(755,apache,apache) %{_localstatedir}/cache/%{metaname}/
%attr(700,apache,apache) %{_sysconfdir}/%{metaname}/Config-web-apache/
%attr(600,apache,apache) %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users
%attr(755,apache,apache) %{_datadir}/%{metaname}/htdocs/protected/Web/Logs

%files api-lighttpd
%defattr(644,root,root)
# Lighttpd logs are stored in /var/log/lighttpd
%attr(755,lighttpd,lighttpd) %{_localstatedir}/cache/%{metaname}/
%attr(700,lighttpd,lighttpd) %{_sysconfdir}/%{metaname}/Config-api-lighttpd/
%attr(600,lighttpd,lighttpd) %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users
%attr(755,lighttpd,lighttpd) %{_datadir}/%{metaname}/htdocs/protected/API/Logs
%{_unitdir}/%{metaname}-api-lighttpd.service
%config(noreplace) %{_sysconfdir}/%{metaname}/%{metaname}-api-lighttpd.conf

%files web-lighttpd
%defattr(644,root,root)
# Lighttpd logs are stored in /var/log/lighttpd
%attr(755,lighttpd,lighttpd) %{_localstatedir}/cache/%{metaname}/
%attr(700,lighttpd,lighttpd) %{_sysconfdir}/%{metaname}/Config-web-lighttpd/
%attr(600,lighttpd,lighttpd) %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users
%attr(755,lighttpd,lighttpd) %{_datadir}/%{metaname}/htdocs/protected/Web/Logs
%{_unitdir}/%{metaname}-web-lighttpd.service
%config(noreplace) %{_sysconfdir}/%{metaname}/%{metaname}-web-lighttpd.conf

%changelog
 * Sat Dec 10 2016 Marcin Haba <marcin.haba@bacula.pl> - 7.5.0-0.1
 - Spec create

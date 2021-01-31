%global langs_api en pl pt ru
%global langs_web en pl pt ja ru
%global destdir build
%global metaname baculum

Summary:	Baculum WebGUI tool for Bacula Community program
Name:		baculum
Version:	9.6.7
Release:	1%{?dist}
License:	AGPLv3
Group:		Applications/Internet
URL:		http://bacula.org
Source0:	bacula-gui-9.6.7.tar.gz
BuildRequires:	make
BuildRequires:	systemd-units
BuildRequires:	selinux-policy
BuildRequires:	selinux-policy-devel
BuildRequires:	checkpolicy
Requires:	php-cli >= 5.4.0
Requires:	php-bcmath
Requires:	php-common
Requires:	php-fpm
Requires:	php-json
Requires:	php-ldap
Requires:	php-mysqlnd
Requires:	php-pdo
Requires:	php-pgsql
Requires:	php-xml
Requires:	policycoreutils-python-utils
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

%description api
This package provides the API files for Baculum.
This module is a part of Baculum.

%package web
Summary:		Baculum API files
Requires:		%name-common = %version-%release
Group:			Applications/Internet

%description web
This package provides the Web files for Baculum.
This module is a part of Baculum.

%package api-httpd
Summary:		Apache configuration for Baculum API
Requires:		%name-api = %version-%release
Group:			Applications/Internet
Requires:		httpd
Requires:		php-bcmath
Requires:		php-cli >= 5.4.0
Requires:		php-common
Requires:		php-json
Requires:		php-fpm
Requires:		php-mysqlnd
Requires:		php-pdo
Requires:		php-pgsql
Requires:		php-xml

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
Requires:		php-bcmath
Requires:		php-cli >= 5.4.0
Requires:		php-common
Requires:		php-json
Requires:		php-mysqlnd
Requires:		php-pdo
Requires:		php-pgsql
Requires:		php-xml
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
Requires:		php-cli >= 5.4.0
Requires:		php-common
Requires:		php-fpm
Requires:		php-json
Requires:		php-ldap
Requires:		php-xml

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
Requires:		php-cli >= 5.4.0
Requires:		php-common
Requires:		php-json
Requires:		php-ldap
Requires:		php-xml
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-web-httpd, %{name}-api-httpd

%description web-lighttpd
This package provides the Lighttpd configuration for Baculum WebGUI.
By using this module it is possible to run Baculum WebGUI in Lighttpd environment.

%package api-selinux
Summary:                SELinux module for Baculum API
Requires:               %name-api = %version-%release
Group:                  Applications/Internet
Requires(post):         policycoreutils-python-utils
Requires(preun):        policycoreutils-python-utils

%description api-selinux
This package provides the SELinux module for Baculum API.
You should install this package if you are using SELinux, that Baculum API
can be run in enforcing mode.

%package web-selinux
Summary:                SELinux module for Baculum Web
Requires:               %name-web = %version-%release
Group:                  Applications/Internet
Requires(post):         policycoreutils-python-utils
Requires(preun):        policycoreutils-python-utils

%description web-selinux
This package provides the SELinux module for Baculum Web.
You should install this package if you are using SELinux, that Baculum Web
can be run in enforcing mode.

%prep
%setup -n bacula-gui-%version/baculum

%build
# Execute files preparation in build directory by Makefile
make build DESTDIR=%{destdir}
# Compilation SELinux policies before loading them
make -C examples/selinux/ -f %{_datadir}/selinux/devel/Makefile %{name}-api.pp
make -C examples/selinux/ -f %{_datadir}/selinux/devel/Makefile %{name}-web.pp
# Remove these cache directories, because here will be symbolic links
rmdir %{destdir}/%{_datadir}/%{metaname}/htdocs/assets
rmdir %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/runtime
for lang in  %{langs_api}; do
	rm %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
done
for lang in  %{langs_web}; do
	rm %{destdir}/%{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
done
rm %{destdir}/%{metaname}-install-checker.sh

%install
cp -ra build/. %{buildroot}
%find_lang %{metaname} --all-name
install -D -m 644 examples/selinux/%{name}-api.pp %{buildroot}%{_datadir}/selinux/packages/%{name}-api/%{name}-api.pp
install -D -m 644 examples/selinux/%{name}-web.pp %{buildroot}%{_datadir}/selinux/packages/%{name}-web/%{name}-web.pp

%post common
if [ $1 -ge 1 ] ; then
    # these symbolic links indicates to Baculum's cache directory
    [ -e %{_datadir}/%{metaname}/htdocs/assets ] ||
	ln -s  %{_localstatedir}/cache/%{metaname} %{_datadir}/%{metaname}/htdocs/assets
    [ -e %{_datadir}/%{metaname}/htdocs/protected/runtime ] ||
	ln -s %{_localstatedir}/cache/%{metaname} %{_datadir}/%{metaname}/htdocs/protected/runtime
fi
if [ $1 -eq 2 ] ; then
    if [ -e %{_datadir}/%{metaname}/htdocs/protected/runtime/application.xml-*/config.cache ] ; then
        rm %{_datadir}/%{metaname}/htdocs/protected/runtime/application.xml-*/config.cache
    fi
fi

%post api-httpd
%systemd_post httpd.service
if [ $1 -eq 1 ] ; then
	ln -s  %{_sysconfdir}/%{metaname}/Config-api-apache %{_datadir}/%{metaname}/htdocs/protected/API/Config
fi

%post api-lighttpd
%systemd_post baculum-api-lighttpd.service
if [ $1 -eq 1 ] ; then
	ln -s  %{_sysconfdir}/%{metaname}/Config-api-lighttpd %{_datadir}/%{metaname}/htdocs/protected/API/Config
fi

%post web-httpd
%systemd_post httpd.service
if [ $1 -eq 1 ] ; then
	ln -s  %{_sysconfdir}/%{metaname}/Config-web-apache %{_datadir}/%{metaname}/htdocs/protected/Web/Config
fi

%post web-lighttpd
%systemd_post baculum-web-lighttpd.service
if [ $1 -eq 1 ] ; then
	ln -s  %{_sysconfdir}/%{metaname}/Config-web-lighttpd %{_datadir}/%{metaname}/htdocs/protected/Web/Config
fi

%post api-selinux
# Write access is possible for web servers user to three directories
# -  Config/ directory stores API settings and web server HTTP Basic credentials
# - /var/cache/baculum - store public assets
# - /var/lib/baculum - debug files and internal runtime files
semanage fcontext -a -t httpd_sys_rw_content_t '%{_sysconfdir}/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R '%{_sysconfdir}/%{name}' || :
restorecon -i -R '%{_datadir}/%{name}/htdocs/protected/API/Config' || :
semanage fcontext -a -t httpd_cache_t '%{_localstatedir}/cache/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R %{_localstatedir}/cache/%{name} || :
semanage fcontext -a -t httpd_sys_rw_content_t '%{_localstatedir}/lib/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R %{_localstatedir}/lib/%{name} || :
semodule -i %{_datadir}/selinux/packages/%{name}-api/%{name}-api.pp 2>/dev/null || :

%post web-selinux
# Write access is possible for web servers user to three directories
# -  Config/ directory stores API settings
# - /var/cache/baculum - store public assets
# - /var/lib/baculum - debug files and internal runtime files
semanage fcontext -a -t httpd_sys_rw_content_t '%{_sysconfdir}/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R '%{_sysconfdir}/%{name}' || :
restorecon -i -R '%{_datadir}/%{name}/htdocs/protected/Web/Config' || :
semanage fcontext -a -t httpd_cache_t '%{_localstatedir}/cache/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R %{_localstatedir}/cache/%{name} || :
semanage fcontext -a -t httpd_sys_rw_content_t '%{_localstatedir}/lib/%{name}(/.*)?' 2>/dev/null || :
restorecon -i -R %{_localstatedir}/lib/%{name} || :
semodule -i %{_datadir}/selinux/packages/%{name}-web/%{name}-web.pp 2>/dev/null || :

%preun common
if [ $1 -lt 1 ] ; then
	rm %{_datadir}/%{metaname}/htdocs/assets
	rm %{_datadir}/%{metaname}/htdocs/protected/runtime
fi

%preun api
if [ $1 -lt 1 ] ; then
	for lang in  %{langs_api}; do
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
	done
	# Clear old debug file to be able to remove directory or link containing it
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log*
	fi
	# In older versions it might be directory, thus check if it is link
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/API/Logs ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs
	elif [ -d %{_datadir}/%{metaname}/htdocs/protected/API/Logs ] ; then
		rmdir %{_datadir}/%{metaname}/htdocs/protected/API/Logs
	fi
fi

%preun web
if [ $1 -lt 1 ] ; then
	for lang in  %{langs_web}; do
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
	done
	# Clear old debug file to be able to remove directory or link containing it
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log*
	fi
	# In older versions it might be directory, thus check if it is link
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/Web/Logs ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs
	elif [ -d %{_datadir}/%{metaname}/htdocs/protected/Web/Logs ] ; then
		rmdir %{_datadir}/%{metaname}/htdocs/protected/Web/Logs
	fi
fi

%preun api-httpd
%systemd_preun httpd.service
if [ $1 -lt 1 ] ; then
	# remove debug files if any
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/API/Config ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Config
	fi
	# Clear old debug files for case changing web server (privileges issue)
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log*
	fi
fi

%preun web-httpd
%systemd_preun httpd.service
if [ $1 -lt 1 ] ; then
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/Web/Config ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Config
	fi
	# Clear old debug files for case changing web server (privileges issue)
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log*
	fi
fi

%preun api-lighttpd
%systemd_preun baculum-api-lighttpd.service
if [ $1 -lt 1 ] ; then
	# stop api service
	/sbin/service baculum-api-lighttpd stop &>/dev/null || :
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/API/Config ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Config
	fi
	# Clear old debug files for case changing web server (privileges issue)
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/API/Logs/baculum-api.log*
	fi
fi

%preun web-lighttpd
%systemd_preun baculum-web-lighttpd.service
if [ $1 -lt 1 ] ; then
	# stop web service
	/sbin/service baculum-web-lighttpd stop &>/dev/null || :
	if [ -L %{_datadir}/%{metaname}/htdocs/protected/Web/Config ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Config
	fi
	# Clear old debug files for case changing web server (privileges issue)
	if [ -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log ] ; then
		rm %{_datadir}/%{metaname}/htdocs/protected/Web/Logs/baculum-web.log*
	fi
fi

%preun api-selinux
if [ $1 -lt 1 ] ; then
	# remove SELinux module
	semodule -r %{name}-api 2>/dev/null || :
fi

%preun web-selinux
if [ $1 -lt 1 ] ; then
	# remove SELinux module
	semodule -r %{name}-web 2>/dev/null || :
fi

%postun api-httpd
%systemd_postun_with_restart httpd.service
if [ $1 -eq 0 ] ; then
	if [ ! -e  %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users -a -e %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users.rpmsave ] ; then
		mv %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users.rpmsave %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users
	fi
fi

%postun web-httpd
%systemd_postun_with_restart httpd.service
if [ $1 -eq 0 ] ; then
	if [ ! -e  %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users -a -e %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users.rpmsave ] ; then
		mv %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users.rpmsave %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users
	fi
fi

%postun api-lighttpd
%systemd_postun_with_restart baculum-api-lighttpd.service
if [ $1 -eq 0 ] ; then
	if [ ! -e  %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users -a -e %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users.rpmsave ] ; then
		mv %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users.rpmsave %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users
	fi
fi

%postun web-lighttpd
%systemd_postun_with_restart baculum-web-lighttpd.service
if [ $1 -eq 0 ] ; then
	if [ ! -e  %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users -a -e %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users.rpmsave ] ; then
		mv %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users.rpmsave %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users
	fi
fi

%posttrans api
# because framework does not use system locale dir, here are linked
# locale files to framework location
for lang in  %{langs_api}; do
	[ -e %{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo ] ||
		ln -s  %{_datadir}/locale/${lang}/LC_MESSAGES/%{metaname}-api.mo \
			%{_datadir}/%{metaname}/htdocs/protected/API/Lang/${lang}/messages.mo
done
# check if exists for case uninstall prev version and install new one
[ -e %{_datadir}/%{metaname}/htdocs/protected/API/Logs ] ||
	ln -s %{_localstatedir}/lib/%{metaname} %{_datadir}/%{metaname}/htdocs/protected/API/Logs

%posttrans web
# because framework does not use system locale dir, here are linked
# locale files to framework location
for lang in  %{langs_web}; do
	[ -e %{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo ] ||
		ln -s  %{_datadir}/locale/${lang}/LC_MESSAGES/%{metaname}-web.mo \
			%{_datadir}/%{metaname}/htdocs/protected/Web/Lang/${lang}/messages.mo
done
# check if exists for case uninstall prev version and install new one
[ -e %{_datadir}/%{metaname}/htdocs/protected/Web/Logs ] ||
	ln -s %{_localstatedir}/lib/%{metaname} %{_datadir}/%{metaname}/htdocs/protected/Web/Logs


%files -f %{metaname}.lang common
%defattr(-,root,root,-)
%dir %{_datadir}/%{metaname}
%dir %{_datadir}/%{metaname}/htdocs
%dir %{_datadir}/%{metaname}/htdocs/protected
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
%defattr(-,root,root,-)
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
%defattr(600,apache,apache,700)
# Apache logs are stored in /var/log/httpd/
%attr(644,-,-) %config(noreplace) %{_sysconfdir}/httpd/conf.d/%{metaname}-api.conf
%attr(750,-,-) %{_localstatedir}/cache/%{metaname}/
%dir %{_sysconfdir}/%{metaname}/Config-api-apache
%config(noreplace) %{_sysconfdir}/%{metaname}/Config-api-apache/%{metaname}.users
%attr(750,-,-) %{_localstatedir}/lib/%{metaname}/

%files web-httpd
%defattr(600,apache,apache,700)
# Apache logs are stored in /var/log/httpd/
%attr(644,-,-) %config(noreplace) %{_sysconfdir}/httpd/conf.d/%{metaname}-web.conf
%attr(750,-,-) %{_localstatedir}/cache/%{metaname}/
%dir %{_sysconfdir}/%{metaname}/Config-web-apache
%config(noreplace) %{_sysconfdir}/%{metaname}/Config-web-apache/%{metaname}.users
%attr(750,-,-) %{_localstatedir}/lib/%{metaname}/

%files api-lighttpd
%defattr(644,lighttpd,lighttpd,700)
# Lighttpd logs are stored in /var/log/lighttpd
%attr(750,-,-) %{_localstatedir}/cache/%{metaname}/
%dir %{_sysconfdir}/%{metaname}/Config-api-lighttpd
%attr(600,-,-) %config(noreplace) %{_sysconfdir}/%{metaname}/Config-api-lighttpd/%{metaname}.users
%attr(-,root,root) %{_unitdir}/%{metaname}-api-lighttpd.service
%config(noreplace) %{_sysconfdir}/%{metaname}/%{metaname}-api-lighttpd.conf
%attr(750,-,-) %{_localstatedir}/lib/%{metaname}/

%files web-lighttpd
%defattr(644,lighttpd,lighttpd,700)
# Lighttpd logs are stored in /var/log/lighttpd
%attr(750,-,-) %{_localstatedir}/cache/%{metaname}/
%dir %{_sysconfdir}/%{metaname}/Config-web-lighttpd
%attr(600,-,-) %config(noreplace) %{_sysconfdir}/%{metaname}/Config-web-lighttpd/%{metaname}.users
%attr(-,root,root) %{_unitdir}/%{metaname}-web-lighttpd.service
%config(noreplace) %{_sysconfdir}/%{metaname}/%{metaname}-web-lighttpd.conf
%attr(750,lighttpd,lighttpd) %{_localstatedir}/lib/%{metaname}/

%files api-selinux
%defattr(-,root,root,-)
%{_datadir}/selinux/packages/%{name}-api/%{name}-api.pp

%files web-selinux
%defattr(-,root,root,-)
%{_datadir}/selinux/packages/%{name}-web/%{name}-web.pp

%changelog
 * Sat Dec 10 2016 Marcin Haba <marcin.haba@bacula.pl> - 7.5.0-0.1
 - Spec create
 * Fri Oct 04 2019 Marcin Haba <marcin.haba@bacula.pl> - 9.6.0
 - Add SELinux support
 * Tue Jun 09 2020 Marcin Haba <marcin.haba@bacula.pl> - 9.6.4
 - Fix PHP dependencies
 - Don't move config files after uninstall
 - Add variable data directory
 - Change cache permissions
 - Change debug directory
 * Sun Oct 25 2020 Marcin Haba <marcin.haba@bacula.pl> - 9.6.6.2
 - Fix php-json dependency for api-httpd and api-lighttpd packages


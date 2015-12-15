%global langs en pl
%global destdir build

Summary:	WebGUI tool for Bacula Community program
Name:		baculum
Version:	7.2.0
Release:	0%{?dist}
License:	AGPLv3
Group:		Applications/Internet
URL:		http://bacula.org/
Source0:	bacula-gui-7.2.0.tar.gz
BuildRequires:	systemd-units
BuildRequires:	selinux-policy
BuildRequires:	selinux-policy-devel
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
current Bacula jobs, media management and others. Baculum has integrated web
console that communicates with Bacula bconsole program.

%package selinux
Summary:		SELinux module for Baculum WebGUI tool
Requires:		%name = %version-%release
Group:			Applications/Internet
Requires(post):		policycoreutils-python
Requires(preun):	policycoreutils-python

%description selinux
This package provides an SELinux module for Baculum WebGUI tool.
You should install this package if you are using SELinux, that Baculum
can be run in enforcing mode.

%package httpd
Summary:		Apache configuration for Baculum WebGUI tool
Requires:		%name = %version-%release
Group:			Applications/Internet
Requires:		httpd
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-lighttpd

%description httpd
This package provides the Apache configuration for Baculum WebGUI tool.
By using this module it is possible to run Baculum in Apache environment.

%package lighttpd
Summary:		Lighttpd configuration for Baculum WebGUI tool
Requires:		%name = %version-%release
Group:			Applications/Internet
Requires:		lighttpd
Requires:		lighttpd-fastcgi
# This conflict field is required because Lighttpd and Apache
# cannot listen on the same port at the same time. Even using diffeernt
# ports cause problems like shared framework cache and
# web server specific directories permissions (for lighttpd and apache
# users).
Conflicts:		%{name}-httpd

%description lighttpd
This package provides the Lighttpd configuration for Baculum WebGUI tool.
By using this module it is possible to run Baculum in Lighttpd environment.

%prep
%setup -n bacula-gui-%version/baculum

%build
# Execute files preparation in build directory by Makefile
make build DESTDIR=%{destdir}
# Compilation SELinuxu policies before loading them
make -C examples/selinux/ -f %{_datadir}/selinux/devel/Makefile %{name}.pp
# Remove these cache directories, because here will be symbolic links
rmdir %{destdir}/%{_datadir}/%{name}/htdocs/assets
rmdir %{destdir}/%{_datadir}/%{name}/htdocs/protected/runtime
for lang in  %{langs}; do
	rm %{destdir}/%{_datadir}/%{name}/htdocs/protected/Lang/${lang}/messages.mo
done

%install
cp -ra build/. %{buildroot}
%find_lang %{name} --all-name
install -m 644 examples/selinux/%{name}.pp %{buildroot}%{_datadir}/selinux/packages/%{name}/

%post
# these symbolic links indicates to Baculum's cache directory
ln -s  %{_localstatedir}/cache/%{name} %{_datadir}/%{name}/htdocs/assets
ln -s  %{_localstatedir}/cache/%{name} %{_datadir}/%{name}/htdocs/protected/runtime
# because framework does not use system locale dir, here are linked
# locale files to framework location
for lang in  %{langs}; do
	ln -s  %{_datadir}/locale/${lang}/LC_MESSAGES/%{name}.mo \
		%{_datadir}/%{name}/htdocs/protected/Lang/${lang}/messages.mo
done

%post selinux
if [ $1 -le 1 ] ; then
	# Write access is possible for web servers user only to two directories
	# -  Data/ directory stores settings and web server HTTP Basic credentials
	# - /var/cache/baculum - cache used by framework in specific locations (assets/ and protected/runtime/)
	#   by symbolic links to cache directory
	semanage fcontext -a -t httpd_sys_rw_content_t '%{_datadir}/%{name}/htdocs/protected/Data(/.*)?' 2>/dev/null || :
	restorecon -i -R '%{_datadir}/%{name}/htdocs/protected/Data' || :
	semanage fcontext -a -t httpd_cache_t '%{_localstatedir}/cache/%{name}(/.*)?' 2>/dev/null || :
	restorecon -i -R %{_localstatedir}/cache/%{name} || :
	semodule -i %{_datadir}/selinux/packages/%{name}/%{name}.pp 2>/dev/null || :
fi

%post httpd
%systemd_post httpd.service
ln -s  %{_sysconfdir}/%{name}/Data-apache %{_datadir}/%{name}/htdocs/protected/Data

%post lighttpd
%systemd_post baculum-lighttpd.service
ln -s  %{_sysconfdir}/%{name}/Data-lighttpd %{_datadir}/%{name}/htdocs/protected/Data

%preun
for lang in  %{langs}; do
	rm %{_datadir}/%{name}/htdocs/protected/Lang/${lang}/messages.mo
done
rm %{_datadir}/%{name}/htdocs/assets
rm %{_datadir}/%{name}/htdocs/protected/runtime


%preun httpd
%systemd_preun httpd.service
if [ $1 -lt 1 ] ; then
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{name}/htdocs/protected/Data/settings.conf ] ||
		mv %{_datadir}/%{name}/htdocs/protected/Data/settings.conf \
		%{_datadir}/%{name}/htdocs/protected/Data/settings.conf.apache
	# remove debug files if any
	[ ! -e %{_datadir}/%{name}/htdocs/protected/Data/baculum.dbg ] ||
		rm %{_datadir}/%{name}/htdocs/protected/Data/baculum*.dbg
	rm %{_datadir}/%{name}/htdocs/protected/Data

fi

%preun lighttpd
%systemd_preun baculum-lighttpd.service
if [ $1 -lt 1 ] ; then
	# Rename settings if exist.
	# Note, 'config' macro cannot be used because this file is created after successful
	# installation via wizard. Also using 'config' macro to /usr location strictly forbidden
	# by Packaging Guidelines.
	[ ! -e %{_datadir}/%{name}/htdocs/protected/Data/settings.conf ] ||
		mv %{_datadir}/%{name}/htdocs/protected/Data/settings.conf \
		%{_datadir}/%{name}/htdocs/protected/Data/settings.conf.lighttpd
	# remove debug files if any
	[ ! -e %{_datadir}/%{name}/htdocs/protected/Data/baculum.dbg ] ||
		rm %{_datadir}/%{name}/htdocs/protected/Data/baculum*.dbg
	rm %{_datadir}/%{name}/htdocs/protected/Data
fi

%postun selinux
if [ $1 -eq 0 ] ; then
	semanage fcontext -d -t httpd_sys_rw_content_t '%{_datadir}/%{name}/htdocs/protected/Data(/.*)?' 2>/dev/null || :
	semanage fcontext -d -t httpd_cache_t '%{_localstatedir}/cache/%{name}(/.*)?' 2>/dev/null || :
	semodule -r %{name} 2>/dev/null || :
fi

%postun httpd
%systemd_postun_with_restart httpd.service

%postun lighttpd
%systemd_postun_with_restart baculum-lighttpd.service

%files -f %{name}.lang
%defattr(-,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{name}/htdocs/protected/Data
%{_datadir}/%{name}
%license LICENSE
%doc AUTHORS README

%files selinux
%defattr(-,root,root)
%{_datadir}/selinux/packages/%{name}/%{name}.pp

%files httpd
%defattr(644,root,root)
# Apache logs are stored in /var/log/httpd/
%config(noreplace) %{_sysconfdir}/httpd/conf.d/%{name}.conf
%attr(755,apache,apache) %{_localstatedir}/cache/%{name}/
%attr(700,apache,apache) %{_sysconfdir}/%{name}/Data-apache/
%attr(600,apache,apache) %{_sysconfdir}/%{name}/Data-apache/%{name}.users

%files lighttpd
%defattr(644,root,root)
# Lighttpd logs are stored in /var/log/lighttpd
%attr(755,lighttpd,lighttpd) %{_localstatedir}/cache/%{name}/
%attr(700,lighttpd,lighttpd) %{_sysconfdir}/%{name}/Data-lighttpd
%attr(600,lighttpd,lighttpd) %{_sysconfdir}/%{name}/Data-lighttpd/%{name}.users
%{_unitdir}/%{name}-lighttpd.service
%config(noreplace) %{_sysconfdir}/%{name}/%{name}-lighttpd.conf

%changelog
 * Tue Dec 15 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.2.0
 - Add creating and removing Data/ directory symbolic link
 - Match locations to bacula-gui directories structure
 * Sat Jul 18 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.5.b
 - Change baculum.users and Data/ directory permissions to more
   restrictive
 - Add noreplace param to Lighttpd config file
 - Add systemd macros for httpd subpackage
 - Fix systemd action in post section
 - Move DESTDIR target and languages to global variables
 - Do not remove settings file when a web server specific package
   is removed (used move action)
 - Drop storing Lighttpd logs in separate logs directory
 - Define locale files
 * Fri Jul 17 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.4.b
 - Remove source files: baculum.users, baculum-apache.conf
   baculum-lighttpd.conf and baculum-lighttpd.service
 - Use reorganized upstream Makefile
 * Tue Jul 14 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.3.b
 - Separate to subpackage Lighttpd support
 - Add Apache subpackage
 - Use upstream Makefile to prepare build files
 - Cache symlbolic links only in install section
 - Add comments to Spec
 - Compile SELinux policies instead of install pre-compiled
 - Add source files: baculum.users, baculum-apache.conf
   baculum-lighttpd.conf and baculum-lighttpd.service
 - Change Source0 URL
 * Mon Jul 13 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.2.b
 - Remove chkconfig and service dependencies from Spec
 - Change Spec sections order
 - Correct package description typos and errors
 * Mon Jul 06 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.1.b
 - Spec create

Summary:	WebGUI tool for Bacula Community program
Name:		baculum
Version:	7.0.6
Release:	0.3.b%{?dist}
License:	AGPLv3
Group:		Applications/Internet
URL:		http://bacula.org/
Source0:	http://bacula.org/downloads/baculum/baculum-7.0.6b.tar.gz
Source1:	baculum.users
Source2:	baculum-apache.conf
Source3:	baculum-lighttpd.conf
Source4:	baculum-lighttpd.service
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
%autosetup

%build
# Execute files preparation in build directory by Makefile
make build
# Compilation SELinuxu policies before loading them
make -C examples/selinux/ -f %{_datadir}/selinux/devel/Makefile %{name}.pp

%install
mkdir -p %{buildroot}%{_datadir}/%{name}/htdocs/protected
mkdir -p %{buildroot}%{_sysconfdir}/%{name}
mkdir -p %{buildroot}%{_sysconfdir}/httpd/conf.d
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}%{_localstatedir}/cache/%{name}
mkdir -p %{buildroot}%{_var}/log/%{name}
mkdir -p %{buildroot}%{_datadir}/selinux/packages/%{name}

cp -ra build/. %{buildroot}%{_datadir}/%{name}/htdocs
install -m 640 %{SOURCE2} %{buildroot}%{_sysconfdir}/httpd/conf.d/%{name}.conf
install -m 640 %{SOURCE3} %{buildroot}%{_sysconfdir}/%{name}/
install -m 644 %{SOURCE4} %{buildroot}%{_unitdir}/
install -m 600 %{SOURCE1} %{buildroot}%{_datadir}/%{name}/htdocs/protected/Data/%{name}.users
install -m 644 examples/selinux/%{name}.pp %{buildroot}%{_datadir}/selinux/packages/%{name}/%{name}.pp
# these symbolic links indicates to Baculum's cache directory
ln -s  %{_localstatedir}/cache/%{name} %{buildroot}%{_datadir}/%{name}/htdocs/assets
ln -s  %{_localstatedir}/cache/%{name} %{buildroot}%{_datadir}/%{name}/htdocs/protected/runtime

%post lighttpd
%systemd_post baculum.service

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

%preun
if [ $1 -lt 1 ] ; then
    # remove settings and logs if exist
    [ ! -e %{_datadir}/%{name}/htdocs/protected/Data/settings.conf ] ||
	rm %{_datadir}/%{name}/htdocs/protected/Data/settings.conf
    [ ! -e %{_datadir}/%{name}/htdocs/protected/Data/baculum.log ] ||
	rm %{_datadir}/%{name}/htdocs/protected/Data/baculum*.log
fi

%preun lighttpd
%systemd_preun baculum-lighttpd.service

%postun lighttpd
%systemd_postun_with_restart baculum-lighttpd.service

%postun selinux
if [ $1 -eq 0 ] ; then
    semanage fcontext -d -t httpd_sys_rw_content_t '%{_datadir}/%{name}/htdocs/protected/Data(/.*)?' 2>/dev/null || :
    semanage fcontext -d -t httpd_cache_t '%{_localstatedir}/cache/%{name}(/.*)?' 2>/dev/null || :
    semodule -r %{name} 2>/dev/null || :
fi

%files
%defattr(-,root,root)
# directory excluded here, because it needs to be provided
# with selected web server privileges (lighttpd or apache)
%exclude %{_datadir}/%{name}/htdocs/protected/Data/
%{_datadir}/%{name}
%license LICENSE
%doc AUTHORS INSTALL README

%files selinux
%defattr(-,root,root)
%{_datadir}/selinux/packages/%{name}/%{name}.pp

%files httpd
%defattr(644,root,root)
# Apache logs are stored in /var/log/httpd/
%config %{_sysconfdir}/httpd/conf.d/%{name}.conf
%attr(700,apache,apache) %{_localstatedir}/cache/%{name}/
%attr(-,apache,apache) %{_datadir}/%{name}/htdocs/protected/Data/

%files lighttpd
%defattr(644,root,root)
# Lighttpd logs are stored in /var/log/baculum
%attr(750,lighttpd,lighttpd) %{_var}/log/%{name}/
%attr(700,lighttpd,lighttpd) %{_localstatedir}/cache/%{name}/
%attr(-,lighttpd,lighttpd) %{_datadir}/%{name}/htdocs/protected/Data/
%{_unitdir}/%{name}-lighttpd.service
%config %{_sysconfdir}/%{name}/%{name}-lighttpd.conf

%changelog
 * Tue Jul 14 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.3.b
 - Separate to subpackage Lighttpd support
 - Add Apache subpackage
 - Use upstream Makefile to prepare build files
 - Cache symlbolic links only in install section
 - Add comments to Spec
 - Compile SELinux policies instead of install pre-compiled
 - Add source files: baculum.users, baculum-apache.conf
   baculum-lighttpd.conf and baculum-lighttpd.service
 * Mon Jul 13 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.2.b
 - Remove chkconfig and service dependencies from Spec
 - Change Spec sections order
 - Correct package description typos and errors
 * Mon Jul 06 2015 Marcin Haba <marcin.haba@bacula.pl> - 7.0.6-0.1.b
 - Spec create

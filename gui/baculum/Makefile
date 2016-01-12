# Makefile to prepare runtime files (for rpm-based and deb-based distributions)
#
# usage: make DESTDIR=/some/runtime_dir

NAME = baculum
VERSION = 7.2.0
DEFTEMPLATE = rpm-template
DEFHTTPDNAME = httpd
DEFHTTPDSITECONF = conf.d
SAMPLETYPE ?= $(DEFTEMPLATE)
HTTPDNAME ?= $(DEFHTTPDNAME)
HTTPDSITECONF ?= $(DEFHTTPDSITECONF)
SYSUSRDIR = /usr
SYSCONFDIR = /etc
SYSVARDIR = /var
CONFDIR = $(SYSCONFDIR)/$(NAME)
HTTPDCONFDIR = $(SYSCONFDIR)/$(HTTPDNAME)/$(HTTPDSITECONF)
UNITDIR = $(SYSUSRDIR)/lib/systemd/system
LOCALEDIR = $(SYSUSRDIR)/share/locale
WWWDIR = $(SYSUSRDIR)/share/$(NAME)/htdocs
SELINUXDIR = $(SYSUSRDIR)/share/selinux/packages/$(NAME)
CACHEDIR = $(SYSVARDIR)/cache/$(NAME)
LOGDIR = $(SYSVARDIR)/log
HTTPDLOGS = $(LOGDIR)/$(HTTPDNAME)
LIGHTTPDLOGS = $(LOGDIR)/lighttpd

# Internal application directories
datadir = protected
frameworkdir = framework
themesdir = themes
cachedir = assets
configdir = Data
configcachedir = $(datadir)/runtime
samplesdir = examples/$(SAMPLETYPE)

datadirsrc = $(datadir)/Class \
	$(datadir)/JavaScript \
	$(datadir)/Layouts \
	$(datadir)/Pages \
	$(datadir)/Portlets

datafilesrc = $(datadir)/application.xml \
	$(datadir)/Init.php

miscfilesrc = index.php \
	AUTHORS \
	INSTALL \
	LICENSE \
	README

localelang = en pl pt
localedirsrc = $(datadir)/Lang
localefile = messages.mo

excluded_dirs = '.*/\(3rdParty\|tinymce-405\).*'
excluded_files = '.*\(\.htaccess\)$$'


build: prepare_build prepare_data prepare_externals prepare_themes prepare_locale setup

prepare_build:
	if [ -z "$(DESTDIR)" ]; then @echo "No DESTDIR given"; exit 1; fi
	mkdir -p $(DESTDIR)$(SYSCONFDIR) \
	$(DESTDIR)$(CONFDIR) \
	$(DESTDIR)$(HTTPDCONFDIR) \
	$(DESTDIR)$(UNITDIR) \
	$(DESTDIR)$(WWWDIR) \
	$(DESTDIR)$(SELINUXDIR) \
	$(DESTDIR)$(CACHEDIR) \
	$(DESTDIR)$(LOCALEDIR) \
	$(DESTDIR)$(WWWDIR)/$(cachedir) \
	$(DESTDIR)$(WWWDIR)/$(configcachedir)
	mkdir -m 700 -p $(DESTDIR)$(CONFDIR)/$(configdir)-apache \
	$(DESTDIR)$(CONFDIR)/$(configdir)-lighttpd

prepare_data: prepare_build
	mkdir -p $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -ra $(datadirsrc) $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -a $(datafilesrc) $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -a $(miscfilesrc) $(DESTDIR)$(WWWDIR)/

prepare_externals: prepare_build
	find $(frameworkdir) -type d ! -regex $(excluded_dirs) -exec mkdir -p $(DESTDIR)$(WWWDIR)/{} \;
	find $(frameworkdir) -type f ! -regex $(excluded_files) -a ! -regex $(excluded_dirs) -exec cp -a {} $(DESTDIR)$(WWWDIR)/{} \;

prepare_themes: prepare_build
	cp -ra $(themesdir) $(DESTDIR)$(WWWDIR)

prepare_locale: prepare_build
	for lang in $(localelang) ; do \
		mkdir -p $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES ; \
		mkdir -p $(DESTDIR)$(WWWDIR)/$(localedirsrc)/$$lang ; \
		install -m 644 $(localedirsrc)/$$lang/$(localefile) $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME).mo ; \
		ln -s $(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME).mo $(DESTDIR)$(WWWDIR)/$(localedirsrc)/$$lang/$(localefile) ; \
	done

prepare_samples:
	install -m 640 $(samplesdir)/$(NAME)-lighttpd.conf $(DESTDIR)$(CONFDIR)
	install -m 644 $(samplesdir)/$(NAME)-lighttpd.service $(DESTDIR)$(UNITDIR)
	install -m 640 $(samplesdir)/$(NAME)-apache.conf $(DESTDIR)$(HTTPDCONFDIR)/$(NAME).conf
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-lighttpd
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-apache
	# Default link apache config file
	ln -s $(DESTDIR)$(CONFDIR)/$(configdir)-apache $(DESTDIR)$(WWWDIR)/$(datadir)/$(configdir)

setup: prepare_samples
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(HTTPDLOGS)#g" $(DESTDIR)$(HTTPDCONFDIR)/$(NAME).conf
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(LIGHTTPDLOGS)#g" $(DESTDIR)$(CONFDIR)/$(NAME)-lighttpd.conf
	sed -i -e "s#%CONFDIR#$(CONFDIR)#g" $(DESTDIR)$(UNITDIR)/$(NAME)-lighttpd.service

# Makefile to prepare runtime files (for rpm-based and deb-based distributions)
#
# usage: make DESTDIR=/some/runtime_dir

NAME = baculum
VERSION = 7.5.0
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
CACHEDIR = $(SYSVARDIR)/cache/$(NAME)
LOGDIR = $(SYSVARDIR)/log
HTTPDLOGS = $(LOGDIR)/$(HTTPDNAME)
LIGHTTPDLOGS = $(LOGDIR)/lighttpd

# Internal application directories
datadir = protected
apidir = API
webdir = Web
commondir = Common
frameworkdir = framework
themesdir = themes
cachedir = assets
configdir = Config
configcachedir = $(datadir)/runtime
samplesdir = examples/$(SAMPLETYPE)

webdatadirsrc = $(datadir)/$(webdir)/Class \
	$(datadir)/$(webdir)/Config \
	$(datadir)/$(webdir)/JavaScript \
	$(datadir)/$(webdir)/Data \
	$(datadir)/$(webdir)/Layouts \
	$(datadir)/$(webdir)/Logs \
	$(datadir)/$(webdir)/Pages \
	$(datadir)/$(webdir)/Portlets

apidatadirsrc = $(datadir)/$(apidir)/Class \
	$(datadir)/$(apidir)/Config \
	$(datadir)/$(apidir)/JavaScript \
	$(datadir)/$(apidir)/Layouts \
	$(datadir)/$(apidir)/Logs \
	$(datadir)/$(apidir)/Pages

commondatadirsrc = $(datadir)/$(commondir)/Class \
	$(datadir)/$(commondir)/Portlets

datafilesrc = $(datadir)/application.xml

webdatafilesrc = $(datadir)/$(webdir)/endpoints.xml \
	$(datadir)/$(webdir)/Init.php

apidatafilesrc = $(datadir)/$(apidir)/endpoints.xml

miscfilesrc = index.php \
	AUTHORS \
	INSTALL \
	LICENSE \
	README

localeweblang = en pl pt ja
localeapilang = en pl
localewebdirsrc = $(datadir)/$(webdir)/Lang
localeapidirsrc = $(datadir)/$(apidir)/Lang
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
	$(DESTDIR)$(CACHEDIR) \
	$(DESTDIR)$(LOCALEDIR) \
	$(DESTDIR)$(WWWDIR)/$(cachedir) \
	$(DESTDIR)$(WWWDIR)/$(configcachedir)
	mkdir -m 700 -p $(DESTDIR)$(CONFDIR)/$(configdir)-api-apache \
	$(DESTDIR)$(CONFDIR)/$(configdir)-api-lighttpd \
	$(DESTDIR)$(CONFDIR)/$(configdir)-web-apache \
	$(DESTDIR)$(CONFDIR)/$(configdir)-web-lighttpd

prepare_data: prepare_build
	mkdir -p $(DESTDIR)$(WWWDIR)/$(datadir) \
	$(DESTDIR)$(WWWDIR)/$(datadir)/$(webdir) \
	$(DESTDIR)$(WWWDIR)/$(datadir)/$(apidir) \
	$(DESTDIR)$(WWWDIR)/$(datadir)/$(commondir)
	cp -ra $(webdatadirsrc) $(DESTDIR)$(WWWDIR)/$(datadir)/$(webdir)
	cp -a $(webdatafilesrc) $(DESTDIR)$(WWWDIR)/$(datadir)/$(webdir)
	cp -ra $(apidatadirsrc) $(DESTDIR)$(WWWDIR)/$(datadir)/$(apidir)
	cp -a $(apidatafilesrc) $(DESTDIR)$(WWWDIR)/$(datadir)/$(apidir)
	cp -ra $(commondatadirsrc) $(DESTDIR)$(WWWDIR)/$(datadir)/$(commondir)
	cp -a $(datafilesrc) $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -a $(miscfilesrc) $(DESTDIR)$(WWWDIR)/

prepare_externals: prepare_build
	find $(frameworkdir) -type d ! -regex $(excluded_dirs) -exec mkdir -p $(DESTDIR)$(WWWDIR)/{} \;
	find $(frameworkdir) -type f ! -regex $(excluded_files) -a ! -regex $(excluded_dirs) -exec cp -a {} $(DESTDIR)$(WWWDIR)/{} \;

prepare_themes: prepare_build
	cp -ra $(themesdir) $(DESTDIR)$(WWWDIR)

prepare_locale: prepare_build
	for lang in $(localeweblang) ; do \
		mkdir -p $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES ; \
		mkdir -p $(DESTDIR)$(WWWDIR)/$(localewebdirsrc)/$$lang ; \
		install -m 644 $(localewebdirsrc)/$$lang/$(localefile) $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME)-web.mo ; \
		ln -s $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME)-web.mo $(DESTDIR)$(WWWDIR)/$(localewebdirsrc)/$$lang/$(localefile) ; \
	done
	for lang in $(localeapilang) ; do \
		mkdir -p $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES ; \
		mkdir -p $(DESTDIR)$(WWWDIR)/$(localeapidirsrc)/$$lang ; \
		install -m 644 $(localeapidirsrc)/$$lang/$(localefile) $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME)-api.mo ; \
		ln -s $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(NAME)-api.mo $(DESTDIR)$(WWWDIR)/$(localeapidirsrc)/$$lang/$(localefile) ; \
	done

prepare_samples:
	install -m 640 $(samplesdir)/$(NAME)-api-lighttpd.conf $(DESTDIR)$(CONFDIR)
	install -m 640 $(samplesdir)/$(NAME)-web-lighttpd.conf $(DESTDIR)$(CONFDIR)
	install -m 644 $(samplesdir)/$(NAME)-api-lighttpd.service $(DESTDIR)$(UNITDIR)
	install -m 644 $(samplesdir)/$(NAME)-web-lighttpd.service $(DESTDIR)$(UNITDIR)
	install -m 640 $(samplesdir)/$(NAME)-api-apache.conf $(DESTDIR)$(HTTPDCONFDIR)/$(NAME)-api.conf
	install -m 640 $(samplesdir)/$(NAME)-web-apache.conf $(DESTDIR)$(HTTPDCONFDIR)/$(NAME)-web.conf
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-api-lighttpd
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-api-apache
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-web-lighttpd
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(CONFDIR)/$(configdir)-web-apache
	# Default link apache config file
	#ln -s $(DESTDIR)$(CONFDIR)/$(configdir)-api-apache $(DESTDIR)$(WWWDIR)/$(datadir)/$(configdir)

setup: prepare_samples
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(HTTPDLOGS)#g" $(DESTDIR)$(HTTPDCONFDIR)/$(NAME)-api.conf
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(HTTPDLOGS)#g" $(DESTDIR)$(HTTPDCONFDIR)/$(NAME)-web.conf
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(LIGHTTPDLOGS)#g" $(DESTDIR)$(CONFDIR)/$(NAME)-api-lighttpd.conf
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(LIGHTTPDLOGS)#g" $(DESTDIR)$(CONFDIR)/$(NAME)-web-lighttpd.conf
	sed -i -e "s#%CONFDIR#$(CONFDIR)#g" $(DESTDIR)$(UNITDIR)/$(NAME)-api-lighttpd.service
	sed -i -e "s#%CONFDIR#$(CONFDIR)#g" $(DESTDIR)$(UNITDIR)/$(NAME)-web-lighttpd.service
	find $(DESTDIR)/ -type f -name .gitignore -exec rm -f {} \;

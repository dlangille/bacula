NAME = baculum
VERSION = 7.0.6b
SAMPLETYPE = rpm-template
SYSUSRDIR = /usr
SYSCONFDIR = /etc
SYSVARDIR = /var
CONFDIR = $(SYSCONFDIR)/$(NAME)
HTTPDCONFDIR = $(SYSCONFDIR)/httpd/conf.d
UNITDIR = $(SYSUSRDIR)/lib/systemd/system
WWWDIR = $(SYSUSRDIR)/share/$(NAME)/htdocs
SELINUXDIR = $(SYSUSRDIR)/share/selinux/packages/$(NAME)
CACHEDIR = $(SYSVARDIR)/cache/$(NAME)
LOGDIR = $(SYSVARDIR)/log
HTTPDLOGS = $(LOGDIR)/httpd
LIGHTTPDLOGS = $(LOGDIR)/$(NAME)

# Internal application directories
datadir = protected
frameworkdir = framework
themesdir = themes
cachedir = assets
configdir = $(datadir)/Data
configcachedir = $(datadir)/runtime
samplesdir = examples/$(SAMPLETYPE)

datadirsrc = $(datadir)/Class \
    $(datadir)/JavaScript \
    $(datadir)/Lang \
    $(datadir)/Layouts \
    $(datadir)/Pages \
    $(datadir)/Portlets

datafilesrc = $(datadir)/.htaccess \
    $(datadir)/application.xml

miscfilesrc = .htaccess \
    index.php \
    AUTHORS \
    INSTALL \
    LICENSE \
    README

build: prepare_build prepare_data prepare_externals prepare_themes setup

prepare_build:
	mkdir -p $(DESTDIR)$(SYSCONFDIR) \
	$(DESTDIR)$(CONFDIR) \
	$(DESTDIR)$(HTTPDCONFDIR) \
	$(DESTDIR)$(UNITDIR) \
	$(DESTDIR)$(WWWDIR) \
	$(DESTDIR)$(SELINUXDIR) \
	$(DESTDIR)$(LIGHTTPDLOGS) \
	$(DESTDIR)$(CACHEDIR)
	mkdir -p -m 700 $(DESTDIR)$(WWWDIR)/$(cachedir) \
	$(DESTDIR)$(WWWDIR)/$(configdir) \
	$(DESTDIR)$(WWWDIR)/$(configcachedir)

prepare_data: prepare_build
	mkdir -p $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -ra $(datadirsrc) $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -a $(datafilesrc) $(DESTDIR)$(WWWDIR)/$(datadir)
	cp -a $(miscfilesrc) $(DESTDIR)$(WWWDIR)/

prepare_externals: prepare_build
	cp -ra $(frameworkdir) $(DESTDIR)$(WWWDIR)

prepare_themes: prepare_build
	cp -ra $(themesdir) $(DESTDIR)$(WWWDIR)

prepare_samples:
	install -m 640 $(samplesdir)/$(NAME)-lighttpd.conf $(DESTDIR)$(CONFDIR)
	install -m 644 $(samplesdir)/$(NAME)-lighttpd.service $(DESTDIR)$(UNITDIR)
	install -m 640 $(samplesdir)/$(NAME)-apache.conf $(DESTDIR)$(HTTPDCONFDIR)/$(NAME).conf
	install -m 600 $(samplesdir)/$(NAME).users $(DESTDIR)$(WWWDIR)/$(configdir)

setup: prepare_samples
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(HTTPDLOGS)#g" $(DESTDIR)$(HTTPDCONFDIR)/$(NAME).conf
	sed -i -e "s#%DOCUMENTROOT#$(WWWDIR)#g" -e "s#%LOGDIR#$(LIGHTTPDLOGS)#g" $(DESTDIR)$(CONFDIR)/$(NAME)-lighttpd.conf
	sed -i -e "s#%CONFDIR#$(CONFDIR)#g" $(DESTDIR)$(UNITDIR)/$(NAME)-lighttpd.service

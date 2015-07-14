
prefix = .
builddir = $(prefix)/build
datadir = protected
frameworkdir = framework
themesdir = themes

datadirsrc = $(datadir)/Class \
    $(datadir)/Data \
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

all: build

build: prepare_build prepare_data prepare_externals prepare_themes


prepare_build:
	[ -d $(builddir) ] || mkdir -p $(builddir)

prepare_data: prepare_build
	[ -d "$(builddir)/$(datadir)" ] || mkdir -p $(builddir)/$(datadir)
	cp -ra $(datadirsrc) $(builddir)/$(datadir)
	cp -a $(datafilesrc) $(builddir)/$(datadir)
	cp -a $(miscfilesrc) $(builddir)

prepare_externals: prepare_build
	cp -ra $(frameworkdir) $(builddir)

prepare_themes: prepare_build
	cp -ra $(themesdir) $(builddir)

install: build
	[ -d $(INSTALL_ROOT) ] || mkdir -p $(INSTALL_ROOT)
	cp -ra $(builddir) $(INSTALL_ROOT)/

#
# Makefile for win32 bacula executables
# Using MinGW cross-compiler on GNU/Linux
#
#  Written by Eric Bollengier, March 2009 
#    based on work by Robert Nelson
#

include ../Makefile.inc

VERSION := $(shell sed -ne 's/^.define[ \t]VERSION[ \t][ \t]*"\(.*\)"/\1/p' < ../../version.h)
RELEASE ?= $(shell awk '/define RELEASE [0-9]+/ { print $$3 }' ../../version.h)

DEFINES := \
	-DVERSION=$(VERSION) \
	-DOUT_DIR=$(BUILDDIR)/release64 \
	-DSRC_DIR=release64 \
	-DSRC64_DIR=$(BUILDDIR)/win64_installer/release64 \
	-DSRC32_DIR=$(BUILDDIR)/win32_installer/release32 \
	-DWINVER=64 \
	-DBUILD_TOOLS=MinGW64 \
	-DPRODUCT_PUBLISHER="Bacula Community" \
	-DBUILD_BAT=$(BUILD_BAT)

INSTALL_EXE := $(BUILDDIR)/release64/winbacula-$(VERSION).exe

BACULABINARIES := \
	bacula.dll \
	alldrives-fd.dll \
	bsleep.exe \
	bsmtp.exe \
	bconsole.exe \
	bacula-fd.exe \
	bacula-sd.exe \
	bextract.exe \
	bls.exe \
	bcopy.exe \
        cdp-fd.dll \
        cdp-client.exe


DEPKGS_BINARIES := \
	openssl.exe \
	sed.exe \
	expr64.exe \
	snooze.exe \
        Qt5Core.dll \
        Qt5Gui.dll  \
        Qt5Network.dll  \
        Qt5Widgets.dll  \
        qwindows.dll \
	libwinpthread-1.dll \
	libgcc_s_seh-1.dll \
	libstdc++-6.dll \
        libcrypto-1_1-x64.dll \
        libssl-1_1-x64.dll \
        zlib1.dll


#	libgcc_s_dw2-1.dll

NONGCC_BINARIES := \
	libmysql.dll

NONGCC_LIBRARIES := \
	libpq.dll

MINGW_BINARIES := 

SCRIPT_FILES := \
	mtx-changer.cmd \
	disk-changer.cmd \
	dvd-handler.cmd

CAT_FILES := \
	create_mysql_database.cmd \
	drop_mysql_database.cmd \
	make_mysql_tables.cmd \
	make_mysql_tables.sql \
	drop_mysql_tables.cmd \
	drop_mysql_tables.sql \
	grant_mysql_privileges.cmd \
	grant_mysql_privileges.sql \
	make_mysql_catalog_backup.cmd \
	create_postgresql_database.cmd \
	create_postgresql_database.sql \
	drop_postgresql_database.cmd \
	make_postgresql_tables.cmd \
	make_postgresql_tables.sql \
	drop_postgresql_tables.cmd \
	drop_postgresql_tables.sql \
	grant_postgresql_privileges.cmd \
	grant_postgresql_privileges.sql \
	make_postgresql_catalog_backup.cmd \
	create_sqlite3_database.cmd \
	drop_sqlite3_database.cmd \
	make_sqlite3_tables.cmd \
	make_sqlite3_tables.sql \
	drop_sqlite3_tables.cmd \
	grant_sqlite3_privileges.cmd \
	make_sqlite3_catalog_backup.cmd \
	delete_catalog_backup.cmd

DIRD_FILES := \
	query.sql

SSL_FILES := \
	openssl.cnf

LICENSE_FILES := \
	LICENSE

ifeq ($(bat),no)
	BACULA_BINARIES=$(BACULABINARIES)
	HELP=
else
	BACULA_BINARIES=$(BACULABINARIES) bat.exe bacula-tray-monitor.exe
	HELP=help
endif


EXTRA=

##########################################################################

# Targets

.PHONY: all clean installer distclean

all:		$(HELP) $(INSTALL_EXE) $(EXTRA)

installer:	$(HELP) $(INSTALL_EXE)

distclean: clean

clean:
	@echo "Cleaning `pwd`"
	$(CMD_ECHO)-rm -f $(INSTALL_EXE)
	$(CMD_ECHO)-rm -rf release64

help:
	rm -rf release64/help
	mkdir -p release64/help
	cp -f $(BINDIR)/help/* release64/help/

#
# Rules
#

define Convert_Binary
release64/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$@
endef

define Copy_Binary
release64/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$@
endef

define Copy_Docs
release64/$(1): $(DOCDIR)/$(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$(dir $$@)
endef

define Copy_Licenses
release64/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$(dir $$@)
endef

$(foreach file,$(addprefix $(DEPKGS)/bin/, $(DEPKGS_BINARIES)),$(eval $(call Convert_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/bin/, $(NONGCC_BINARIES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/lib/, $(NONGCC_LIBRARIES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(BINDIR)/, $(BACULA_BINARIES)),$(eval $(call Convert_Binary,$(file))))

$(foreach file,$(addprefix $(MINGW_DLLDIR)/, $(MINGW_BINARIES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/ssl/, $(SSL_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../scripts/, $(SCRIPT_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../cats/, $(CAT_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../../dird/, $(DIRD_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(MAINDIR)/, $(LICENSE_FILES)),$(eval $(call Copy_Licenses,$(file))))

#$(foreach file,$(DOC_FILES),$(eval $(call Copy_Docs,$(file))))

$(INSTALL_EXE): winbacula.nsi $(addprefix release64/,$(BACULA_BINARIES) $(DEPKGS_BINARIES) $(SSL_FILES) $(LICENSE_FILES))
	makensis -V3 $(DEFINES) winbacula.nsi
	echo " "

include $(BUILDDIR)/Makefile.rules

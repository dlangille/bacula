#
# Copyright (C) 2000-2018 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
#
#  Makefile for Win32/Win64 Bacula File daemon
#
#  If called make win64=yes the Win64 version will be built
#     otherwise the Win32 version will be built.
#
#  If called with bat=no, bat will not be built, otherwise it will be
#     built.
#

ECHO_CMD=@

WIN32_DIRS=lib filed filed/plugins console scripts stored
WIN64_DIRS=lib filed filed/plugins console scripts stored

ifeq ($(win64),yes)
	 DIRS=$(WIN64_DIRS)
	 INSTALLER=win64_installer
	 SPECIAL=bat64 bsmtp
else
	 DIRS=$(WIN32_DIRS)
	 INSTALLER=win32_installer
	 SPECIAL=bat32 bsmtp
endif

ifeq ($(bat),no)
	 SPECIAL=bsmtp
endif	

.PHONY: $(DIRS) clean all Makefile.inc win32_installer full_win32_installer \
	 win64_installer bsmtp distclean

all: Makefile.inc $(DIRS) $(SPECIAL) $(INSTALLER)

distclean: clean

clean: $(DIRS) win32_installer win64_installer
	$(MAKE) -C tools clean
	$(ECHO_CMD)-rm -rf release32 release64
	$(ECHO_CMD)-rm -rf ../qt-console/obj32 ../qt-console/obj64
	$(ECHO_CMD)-rm -f ../qt-console/release/bat.exe ../qt-console/debug/bat.exe
	$(ECHO_CMD)-rm -rf ../qt-console/tray-monitor/obj32 ../qt-console/tray-monitor/obj64
	$(ECHO_CMD)-rm -f ../qt-console/tray-monitor/release/bacula-tray-monitor.exe ../qt-console/tray-monitor/debug/bacula-tray-monitor.exe

is_depkgs_set:
	@if test -z $${DEPKGS} ; then \
	   echo "variable DEPKGS not set" ; \
	   echo "You probably want DEPKGS=\"`(cd ../../..;pwd)`\"" ; \
	   exit 1; \
	fi ; \

release32/bat.exe: is_depkgs_set
	( cd ../qt-console; ./make-win32 "32" )
	test -f release32/bat.exe

bat32: release32/bat.exe

release64/bat.exe: is_depkgs_set
	( cd ../qt-console; ./make-win32 "64" )
	test -f release64/bat.exe

bat64: release64/bat.exe

full_win32_installer:
	@if test -f Makefile.inc; then \
	   if $(MAKE) -C $@ $(MAKECMDGOALS); then \
		echo "\n===== Make of $@ succeeded =====\n\n" ; \
	   else  \
		echo "\n!!!!! Make of $@ failed !!!!!\n\n" ; exit 1; \
	   fi ; \
	fi

win32_installer:
	echo "Making 32 bit version"
	@if test -f Makefile.inc; then \
	   if $(MAKE) -C $@ $(MAKECMDGOALS); then \
		echo "\n===== Make of $@ succeeded =====\n\n" ; \
	   else  \
		echo "\n!!!!! Make of $@ failed !!!!!\n\n" ; exit 1; \
	   fi ; \
	fi

win64_installer:
	echo "Making 64 bit version"
	@if test -f Makefile.inc; then \
	   if $(MAKE) -C $@ $(MAKECMDGOALS); then \
		echo "\n===== Make of $@ succeeded =====\n\n" ; \
	   else  \
		echo "\n!!!!! Make of $@ failed !!!!!\n\n" ; exit 1; \
	   fi ; \
	fi


$(DIRS):
	@if test -f Makefile.inc; then \
	   if $(MAKE) -C $@ $(MAKECMDGOALS); then \
		echo "\n===== Make of $@ succeeded =====\n\n" ; \
	   else  \
		echo "\n!!!!! Make of $@ failed !!!!!\n\n" ; exit 1; \
	   fi ; \
	fi

bsmtp:
	@if test -f Makefile.inc; then \
	   if $(MAKE) -C tools bsmtp $(MAKECMDGOALS); then \
		echo "\n===== Make of tools/bsmtp succeeded =====\n\n" ; \
	   else  \
		echo "\n!!!!! Make of tools/bsmtp failed !!!!!\n\n" ; exit 1; \
	   fi ; \
	fi



Makefile.inc:
	@echo Creating $@
	$(ECHO_CMD)TOPDIR=`(cd ../../..;pwd)`; \
	TOPDIR=$${DEPKGS:-$${TOPDIR}}; \
	if test "$(win64)" = yes -a -e $${TOPDIR}/cross-tools/mingw-w64/bin/x86_64-pc-mingw32-gcc; then \
		BINDIR=$${TOPDIR}/cross-tools/mingw-w64/bin; \
		INCDIR=$${TOPDIR}/cross-tools/mingw-w64/x86_64-pc-mingw32/include; \
		DLLDIR=$${TOPDIR}/cross-tools/mingw-w64/x86_64-pc-mingw32/bin; \
		DEPKGSDIR=depkgs-mingw-w64; \
		DEPKGSDIR32=depkgs-mingw32; \
		MINGWBASE=x86_64-pc-mingw32; \
	elif test -e $${TOPDIR}/cross-tools/mingw32/bin/mingw32-gcc; then \
		BINDIR=$${TOPDIR}/cross-tools/mingw32/bin; \
		INCDIR=$${TOPDIR}/cross-tools/mingw32/mingw32/include; \
		DLLDIR=$${TOPDIR}/cross-tools/mingw32/mingw32/bin; \
		DEPKGSDIR=depkgs-mingw32; \
		DEPKGSDIR32=depkgs-mingw32; \
		MINGWBASE=mingw32; \
	elif test "$(win64)" = yes && which x86_64-w64-mingw32-g++ ; then \
		BINDIR=; \
		INCDIR=; \
		DLLDIR=; \
		DEPKGSDIR=depkgs-mingw-w64; \
		DEPKGSDIR32=depkgs-mingw32; \
		MINGWBASE=x86_64-w64-mingw32; \
	elif which i686-w64-mingw32-g++ ; then \
		BINDIR=; \
		INCDIR=; \
		DLLDIR=; \
		DEPKGSDIR=depkgs-mingw32; \
		DEPKGSDIR32=depkgs-mingw32; \
		MINGWBASE=i686-w64-mingw32; \
	else \
		echo "\nThe GCC cross compiler isn't installed."; \
		echo "You must run build-win32-cross-tools and build-dependencies first.\n"; \
		exit 1; \
	fi ; \
	BUILDDIR=`(pwd)`; \
	MAINDIR=`(cd ../..;pwd)`; \
	sed \
		-e "s^@MINGWBASE@^$${MINGWBASE}^" \
		-e "s^@WIN64@^$${win64}^" \
		-e "s^@BAT@^$${bat}^" \
		-e "s^@WIN32DEPKGS@^$${DEPKGSDIR}^" \
		-e "s^@WIN32DEPKGS32@^$${DEPKGSDIR32}^" \
		-e "s^@WIN32BUILDDIR@^$${BUILDDIR}^" \
		-e "s^@WIN32MAINDIR@^$${MAINDIR}^" \
		-e "s^@WIN32TOPDIR@^$${TOPDIR}^" \
		-e "s^@WIN32BINDIR@^$${BINDIR}^" \
		-e "s^@WIN32INCDIR@^$${INCDIR}^" \
		-e "s^@WIN32DLLDIR@^$${DLLDIR}^" < Makefile.inc.in > $@

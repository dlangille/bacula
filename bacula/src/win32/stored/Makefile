#
# Makefile for win32 bacula executables
# Using MinGW cross-compiler on GNU/Linux
#
#
# Author: Robert Nelson
# License: BSD 2-Clause; see file LICENSE-FOSS
#
#  Written by Robert Nelson, June 2006
#

# Configuration

include ../Makefile.inc

INCLUDES = \
	-I. \
	-I$(MAINDIR)/src/stored \
	$(INCLUDE_DDK) \
	$(INCLUDE_PTHREADS) \
	$(INCLUDE_BACULA) \
	$(INCLUDE_ZLIB) \
	$(INCLUDE_OPENSSL) \
	$(INCLUDE_ICONS)

DEFINES = \
	-DUSING_DLL \
	-DWINVER=0x500 \
	$(HAVES)

#	-DHAVE_TRAY_MONITOR \


vpath %.c $(MAINDIR)/src/stored $(BUILDDIR)/libwin32
vpath %.cpp $(MAINDIR)/src/stored $(BUILDDIR)/libwin32

##########################################################################

COMMON_OBJS = \
	$(OBJDIR)/askdir.o \
	$(OBJDIR)/tape_alert.o \
	$(OBJDIR)/vtape_dev.o \
	$(OBJDIR)/tape_dev.o \
	$(OBJDIR)/init_dev.o \
	$(OBJDIR)/global.o \
	$(OBJDIR)/acquire.o \
	$(OBJDIR)/aligned_dev.o \
	$(OBJDIR)/aligned_read.o \
	$(OBJDIR)/aligned_write.o \
	$(OBJDIR)/ansi_label.o \
	$(OBJDIR)/authenticate.o \
	$(OBJDIR)/autochanger.o \
	$(OBJDIR)/block.o \
	$(OBJDIR)/block_util.o \
	$(OBJDIR)/butil.o \
	$(OBJDIR)/dev.o \
	$(OBJDIR)/device.o \
	$(OBJDIR)/ebcdic.o \
	$(OBJDIR)/hello.o \
	$(OBJDIR)/label.o \
	$(OBJDIR)/lock.o \
	$(OBJDIR)/match_bsr.o \
	$(OBJDIR)/mount.o \
	$(OBJDIR)/parse_bsr.o \
	$(OBJDIR)/read_records.o \
	$(OBJDIR)/record_read.o \
	$(OBJDIR)/record_util.o \
	$(OBJDIR)/record_write.o \
	$(OBJDIR)/reserve.o \
	$(OBJDIR)/scan.o \
	$(OBJDIR)/spool.o \
	$(OBJDIR)/stored_conf.o \
	$(OBJDIR)/vol_mgr.o \
	$(OBJDIR)/wait.o \
	$(OBJDIR)/file_dev.o \
	$(OBJDIR)/sd_plugins.o \
	$(OBJDIR)/os.o



#	 $(OBJDIR)/mtops.o

# bacula-sd
SD_OBJS = \
	$(OBJDIR)/stored.o \
	$(OBJDIR)/append.o \
	$(OBJDIR)/dircmd.o \
	$(OBJDIR)/fd_cmds.o \
	$(OBJDIR)/job.o \
	$(OBJDIR)/read.o \
	$(OBJDIR)/vbackup.o \
	$(OBJDIR)/status.o

# bextract
BEXTRACT_OBJS = \
	$(OBJDIR)/bextract.o 

OBJS_WIN = \
	$(OBJDIR)/service.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/bacula.res

#	$(OBJDIR)/trayMonitor.o \
#	$(OBJDIR)/aboutDialog.o \
#	$(OBJDIR)/statusDialog.o \


ALL_OBJS = \
	$(COMMON_OBJS) \
	$(SD_OBJS) \
	$(BEXTRACT_OBJS) \
	$(OBJS_WIN) \
	$(OBJDIR)/bcopy.o \
	$(OBJDIR)/bls.o \
	$(OBJDIR)/btape.o \
	$(OBJDIR)/bscan.o

LIBS_STORED = \
	$(LIBS_PTHREADS) \
	$(LIBS_NETWORK) \
	-lole32 \
	-loleaut32 \
	-luuid \
	-lcomctl32

######################################################################

# Targets

.PHONY: all clean

all:	$(BINDIR)/bacula-sd.exe  $(BINDIR)/bcopy.exe $(BINDIR)/bextract.exe  $(BINDIR)/bls.exe 
# $(BINDIR)/bscan.exe $(BINDIR)/btape.exe

clean:
	@echo "Cleaning `pwd`"
	$(call clean_obj,$(ALL_OBJS))
	$(call clean_exe,$(BINDIR)/bacula-sd.exe)
	$(call clean_exe,$(BINDIR)/bcopy.exe)
	$(call clean_exe,$(BINDIR)/bextract.exe)
	$(call clean_exe,$(BINDIR)/bls.exe)
	$(call clean_exe,$(BINDIR)/bscan.exe)
	$(call clean_exe,$(BINDIR)/btape.exe)
	$(ECHO_CMD)rm -rf $(OBJDIRS)

realclean: clean
	rm -f tags bacula-sd.conf

#
# Rules
#

$(BINDIR)/bacula-sd.exe: $(SD_OBJS) $(COMMON_OBJS) $(OBJS_WIN) $(LIBS_BACULA)
	$(call link_winapp,$(LIBS_STORED))

$(BINDIR)/btape.exe: $(OBJDIR)/btape.o $(COMMON_OBJS) $(LIBS_BACULA)
	$(call link_conapp,$(LIBS_STORED))

$(BINDIR)/bls.exe: $(OBJDIR)/bls.o $(COMMON_OBJS) $(LIBS_BACULA)
	$(call link_conapp,$(LIBS_STORED))

$(BINDIR)/bextract.exe: $(BEXTRACT_OBJS) $(COMMON_OBJS) $(LIBS_BACULA) $(LIBS_LZO)
	$(call link_conapp,$(LIBS_STORED) $(LIBS_ZLIB))

$(BINDIR)/bscan.exe: $(OBJDIR)/bscan.o $(COMMON_OBJS) $(LIBS_CATS) $(LIBS_BACULA)
	$(call link_conapp,$(LIBS_STORED))

$(BINDIR)/bcopy.exe: $(OBJDIR)/bcopy.o $(COMMON_OBJS) $(LIBS_BACULA)
	$(call link_conapp,$(LIBS_STORED))

$(OBJDIR)/mtops.o: mtops.cpp
	@echo "Compiling $@"
	$(call checkdir,$@)
	$(ECHO_CMD)$(CXX) $(CFLAGS) $(INCLUDE_DDK) -I../../stored -c $< -o $@

$(OBJDIR)/winres.res:	  baculasd/winres.rc
	@echo "Compiling $@"
	$(call checkdir,$@)
	$(ECHO_CMD)$(WINDRES) $(INCLUDE_ICONS) -I baculasd -O coff $< -o $@

include ../Makefile.rules

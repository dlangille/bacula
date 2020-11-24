#
# Copyright (C) 2000-2019 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#

RMF = /bin/rm -f
CXX = /usr/bin/g++
NO_ECHO = @

#
# Flags
#

CXXFLAGS = -g -O2 -Wall

#
# Paths 
#

COMMON_PATH = ../common
BUILD_PATH = ../../build
SRC_PATH = $(BUILD_PATH)/src
JOURNAL_PATH = $(SRC_PATH)/plugins/fd
FD_PLUGIN_PATH = $(SRC_PATH)/plugins/fd
FILED_PATH = $(SRC_PATH)/filed 
CLIENT_PATH = $(SRC_PATH)/tools/cdp-client

BIN_DIR = bin

#
# Libs and Includes
#

LIBS = -L$(SRC_PATH)/lib -L$(SRC_PATH)/findlib -lbac -lbacfind -lpthread -ldl

INCLUDES = -I. -I$(COMMON_PATH) -I$(SRC_PATH) -I$(CLIENT_PATH) -I$(JOURNAL_PATH) -I$(SRC_PATH)/lib -I$(SRC_PATH)/findlib -I $(FILED_PATH)

#
# Libtool specific settings
#

LIBTOOL = $(BUILD_PATH)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --silent --tag=CXX --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --silent --tag=CXX --mode=link
LIBTOOL_CLEAN = $(LIBTOOL) --silent --tag=CXX --mode=clean

.SUFFIXES: .c .cpp .lo

# inference rules
.c.lo:
	@echo "Compiling $<"
	$(NO_ECHO) $(LIBTOOL_COMPILE) $(CXX) -c $(CPPFLAGS) $(INCLUDES) $<
.cpp.lo:
	@echo "Compiling $<"
	$(NO_ECHO) $(LIBTOOL_COMPILE) $(CXX) -c $(CPPFLAGS) $(INCLUDES) $<

#-------------------------------------------------------------------------

all: Makefile test-dirs journal-test folderwatcher-test backupservice-test cdp-plugin-test
	mkdir -p $(BIN_DIR)
	@echo "==== Make of cdp-tests is good ===="
	@echo " "

test-dirs:
	mkdir -p $(BIN_DIR)

run-all: all
	$(BIN_DIR)/journal-test
	$(BIN_DIR)/folderwatcher-test
	$(BIN_DIR)/backupservice-test
	$(BIN_DIR)/cdp-plugin-test

clean: 
	@$(RMF) -r $(BIN_DIR) .libs _libs ./tmp ./*.lo 

JTEST_OBJS = journal-test.lo $(JOURNAL_PATH)/journal.lo
journal-test: Makefile $(JTEST_OBJS) 
	$(LIBTOOL_LINK) $(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $(JTEST_OBJS) $(LIBS) 

FTEST_OBJS = folderwatcher-test.lo $(CLIENT_PATH)/folderwatcher.lo
folderwatcher-test: Makefile $(FTEST_OBJS) 
	$(LIBTOOL_LINK) $(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $(FTEST_OBJS) $(LIBS) 

BTEST_OBJS = backupservice-test.lo $(JOURNAL_PATH)/journal.lo $(CLIENT_PATH)/folderwatcher.lo $(CLIENT_PATH)/backupservice.lo
backupservice-test: Makefile $(BTEST_OBJS) 
	$(LIBTOOL_LINK) $(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $(BTEST_OBJS) $(LIBS) 

PTEST_OBJS = cdp-plugin-test.lo $(JOURNAL_PATH)/journal.lo
cdp-plugin-test: Makefile $(PTEST_OBJS) 
	$(LIBTOOL_LINK) $(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $(PTEST_OBJS) $(LIBS) 


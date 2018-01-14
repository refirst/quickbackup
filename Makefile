#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -O \
  -Wall \
  -fPIC
CFLAGS=-g \
  -pipe \
  -W \
  -O \
  -Wall \
  -fPIC
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS
CUR_PATH=./

#============ CCP vars ============
qbobject=qb.o \
    qb_util.o \
    qb_option.o \
    qb_daemon.o \
    qb_log.o \
    qb_lock.o \
    qb_backup.o

.PHONY:all
all: quickbackup
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:clean
clean:
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf quickbackup
	rm -rf *.o

quickbackup:$(qbobject)
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mquickbackup[0m']"
	$(CXX) $(qbobject) \
    -Xlinker "-(" \
    -lpthread \
    -Xlinker "-)" \
    -o quickbackup

#========================comsync===========================================
qb.o:qb.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb.o qb.cpp

qb_util.o:qb_util.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_util.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_util.o qb_util.cpp

qb_option.o:qb_option.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_option.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_option.o qb_option.cpp

qb_daemon.o:qb_daemon.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_daemon.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_daemon.o qb_daemon.cpp

qb_log.o:qb_log.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_log.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_log.o qb_log.cpp

qb_lock.o:qb_lock.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_lock.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_lock.o qb_lock.cpp

qb_backup.o:qb_backup.cpp
	@echo "[[1;32;40mMAKE:BUILD[0m][Target:'[1;32;40mqb_backup.o[0m']"
	$(CXX) -c -I$(CUR_PATH) $(CPPFLAGS) $(CXXFLAGS) -o qb_backup.o qb_backup.cpp

endif #ifeq ($(shell uname -m),x86_64)

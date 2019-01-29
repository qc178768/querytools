#include option.mak
PREFIX = /opt/utoss/querytools/cdrquery
#This part must be Configured according to actual situations
#module output directory
TARGET_DIR 	= .
#module output name
TARGET_NAME = query
#reference include file path, separate by space
INCLUDE_DIR	=  -I../include 
#define macro
MACROS 		= -DLINUX -D__GNU__
#operate system platform option
#PLATFORM = -m32


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %cpp, %o, $(SRCS))
	
TARGET = ${TARGET_DIR}/${TARGET_NAME}

#----------------------------------------#
CXX 		:= g++ 
ECHO		:= echo
RM			:= rm -rf
MKDIR		:= mkdir -p
CXXFLAGS	:= -g -Wall
CPPFLAGS	:= ${MACROS} ${INCLUDE_DIR} 
#----------------------------------------#

all: ${TARGET}

env:
	@${ECHO} TARGET=${TARGET}
	@${ECHO} CXXFLAGS=${CXXFLAGS}
	@${ECHO} CPPFLAGS=${CPPFLAGS}

%.o:%.cpp
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $< -o $@

#compile app
${TARGET}:${OBJS}
	${CXX} $^  ${CXXFLAGS} -o $@
	
install:
	
	${MKDIR} ${PREFIX}/bin
	/usr/bin/install -m 0755 	${TARGET} ${PREFIX}/bin
	chmod -R 0755 ${PREFIX}
	chown -R utoss ${PREFIX}

clean:
	${RM} ${OBJS}  ${TARGET} ${PREFIX}/bin 
	

.PHONY : all clean env  install 

# MakeFile
# xstress - xk0derz SMTP Stress Tester
#
# (c) Amit Singh, amit@xkoder.com
# http://xkoder.com
#
# This software and related files are licensed under GNU GPL version 2
# Please visit the following webpage for more details
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or read the accompanying LICENSE file.
# 
# View the README file for usage directions
# and help of editing the configuration file
#

OBJ_DIR=obj
SRC_DIR=src
INCLUDE_DIR=include
BIN_DIR=bin

OBJS=xstress.o \
	 sendmail.o \
     config.o \
     thread.o \
     logger.o \
     userinterface.o

SRC=$(addsuffix .cc, $(basename $(OBJS)))
OUT_OBJS=$(addprefix $(OBJ_DIR)/,$(OBJS))
IN_SRC=$(addprefix $(SRC_DIR)/,$(SRC))

B64_SRC=$(SRC_DIR)/b64.c
B64_OUT=$(BIN_DIR)/base64

PROG=$(BIN_DIR)/xstress

GPP=g++
GCC=gcc
LD=ld


.PHONY: all clean mkobjdir

all: mkobjdir $(B64_OUT) $(PROG)
	cp -i xstress.conf $(BIN_DIR)

$(PROG): $(OUT_OBJS)
	$(GPP) $(OUT_OBJS) -o $(PROG)

$(B64_OUT): $(BASE_SRC)
	$(GCC) $(B64_SRC) -o $(B64_OUT)

%.o:
	$(GPP) -c $(subst $(OBJ_DIR), $(SRC_DIR), $(addsuffix .cc, $(basename  $@))) -o $@ -I${INCLUDE_DIR}

mkobjdir:
	if [ -d ${OBJ_DIR} ]; then echo ""; else mkdir ${OBJ_DIR}; fi


clean:
	rm -f *.o
	rm -f $(OUT_OBJS)
	rm -f $(PROG)
	rm -f $(B64_OUT)

install: 
	cp xstress /usr/local/bin

uninstall:
	rm -i /usr/local/bin/xstress 

# MakeFile
# xstress - xk0derz SMTP Stress Tester
#
# (c) Amit Singh, amit@xkoder.com
# http://xkoder.com
#
# This software and related files are licensed under GNU GPL version 2
# Please visit the following webpage for more details
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or read the accopanying LICENSE file.
# 
# View the README file for usage directions
# and help of editing the configuration file
#

OBJ_DIR=obj
SRC_DIR=src
INCLUDE_DIR=include
BIN_DIR=bin

OBJS=$(OBJ_DIR)/xstress.o $(OBJ_DIR)/sendmail.o \
     $(OBJ_DIR)/config.o \
     $(OBJ_DIR)/thread.o \
     $(OBJ_DIR)/logger.o \
     $(OBJ_DIR)/userinterface.o

SRC=$(addsuffix .cc, $(basename $(subst $(OBJ_DIR),$(SRC_DIR),$(OBJS))))

B64_SRC=$(SRC_DIR)/b64.c
B64_OUT=$(BIN_DIR)/base64

PROG=$(BIN_DIR)/xstress

GPP=g++
GCC=gcc
LD=ld


.PHONY: all
.PHONY: clean

all: $(B64_OUT) $(PROG)
	cp -i xstress.conf $(BIN_DIR)

$(OBJS): $(SRC)
	$(GPP) -c $(SRC) -I $(INCLUDE_DIR)
	mv *.o $(OBJ_DIR)

.o.c:
	$(GPP) -c

$(B64_OUT): $(BASE_SRC)
	$(GCC) $(B64_SRC) -o $(B64_OUT)

$(PROG): $(OBJS)
	$(GPP) -o $(PROG) $(OBJS)

clean:
	rm -f *.o
	rm -f $(OBJS)
	rm -f $(PROG)
	rm -f $(B64_OUT)

install: 
	cp xstress /usr/local/bin

uninstall:
	rm -i /usr/local/bin/xstress 

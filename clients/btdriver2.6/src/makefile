#*****************************************************************************
#*
#*    Filename: makefile
#*
#*    Purpose:  To compile the Model 18-902 ./src directory software
#*
#*    $Revision$
#*
#*****************************************************************************
#*****************************************************************************
#*
#*        Copyright (c) 1997 by SBS Technologies, Inc.
#*                     All Rights Reserved.
#*              License governs use and distribution.
#*
#*****************************************************************************

#*****************************************************************************
#*    Makefile macros
#*****************************************************************************

# This is required to identify what Bit 3 Software Product is being used
EXTRA_DEFINE=-DBT1003
CC_OPTIM=-O2
ADDED_CFLAGS=-g -DDEBUG
#ADDED_CFLAGS=-DNDEBUG

# Location of Bit 3 Mirror API header files
BTINCLUDE=../include

# You need to set these to reference the Bit3 ./lib directory so that
# the tools can find the library files and header files

LIB_EXTRA=-L$(BTINCLUDE) -lbtp -lpthread 
EXTRA_INCLUDE=

CPPFLAGS =-I. -I$(BTINCLUDE) -I/usr/src/linux/include $(EXTRA_DEFINE) -D_POSIX_C_SOURCE -D_REENTRANT

CFLAGS=$(CC_OPTIM) $(ADDED_CFLAGS) -Wall -Wstrict-prototypes


#*****************************************************************************
#* Targets
#*****************************************************************************

SOURCES = readmem.c dumpmem.c datachk.c bt_info.c bt_icbr.c bt_cat.c bt_reset.c bt_revs.c bt_bind.c
EXAMPLES = $(SOURCES:%.c=%)

#*****************************************************************************
#* Dependencies
#*****************************************************************************

exe all: lib $(EXAMPLES)

%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LIB_EXTRA)
lib:
	cd ../lib; make; cd ../src
clean:
	$(RM) *.o $(EXAMPLES)

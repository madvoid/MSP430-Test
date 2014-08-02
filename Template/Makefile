# Makefile
#
# ****************************************************************************************************
# Author:
#	Nipun Gunawardena
#
# Credits:
#	Florian Heller's Makefile - <flo@cs.rwth-aachen.de>
#
# Requirements:
#	None
#
# Description:
#	Simple makefile for use with TI MSP430 breakout
#
# ****************************************************************************************************


# ----------------------------------------------------------------------------------------------------
# Project properties
# ----------------------------------------------------------------------------------------------------
FILENAME = template
DEVICE = msp430g2553
EXTERN_FILES =




# ----------------------------------------------------------------------------------------------------
# Definitions
# ----------------------------------------------------------------------------------------------------

# Compiler Prefix
PREFIX = msp430

# Compiler, dissasembler, etc.
CC = ${PREFIX}-gcc
OBJCOPY = ${PREFIX}-objcopy
OBJDUMP = ${PREFIX}-objdump

# Compiler flags
CFLAGS = -mmcu=${DEVICE} -O2 -Wall -g

# Objectcopy flags
CPFLAGS = -O

# Objectdump flags
ODFLAGS = -dST

# Files
SRC = ${wildcard *.c} ${EXTERN_FILES}
OBJS = ${SRC:.c=.o}




# ----------------------------------------------------------------------------------------------------
# Rules
# ----------------------------------------------------------------------------------------------------
all: ${OBJS} ${FILENAME}.elf ${FILENAME}

%.o: %.c ${EXTERN_FILES}
	@echo Compiling ${<}...
	@${CC} ${CFLAGS} -c ${<} -o ${@}
	
${FILENAME}.elf: ${OBJS}
	@echo Linking...
	@${CC} ${CFLAGS} -o ${FILENAME}.elf ${OBJS}

${FILENAME}: ${FILENAME}.elf
	@echo Copying...
	@${OBJCOPY} ${CPFLAGS} ihex ${FILENAME}.elf ${FILENAME}.a43
#@echo Dumping...
#${OBJDUMP} ${ODFLAGS} ${FILENAME}.elf > ${FILENAME}.lst

upload: ${FILENAME}.elf
	mspdebug rf2500 "prog ${FILENAME}.elf"

clean:
	rm -fv *.hex *.elf *.a43 *.lst *.o *.d

devices:
	@${CC} --target-help

#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#
# Included originally in the yagarto projects. Original Author : Michael Fischer
# Modified to suit our purposes by Hussam Al-Hertani
# Use at your own risk!!!!!
##############################################################################################
# Start of default section
#
#CC   = $(CCPREFIX)gcc -g
CP   = $(CCPREFIX)objcopy
AS   = $(CCPREFIX)gcc -x assembler-with-cpp
INSTALL=install

ifdef PROJECT_VERSION
override PROJECT_VERSION := -DPROJECT_VERSION=$(PROJECT_VERSION)
endif

# standard version
#MODBUS_LIBDIR = /usr/lib/modbus
#INCDIRS = /usr/include/modbus

# But we need libmodbus version >= 3.1.4
#   - older versions listen on all interface
#   - there was problem with tty name length

DSTDIR = .


# List all user libraries here
#LIBS = libmodbus m
LIBS = m

#list of src files to include in build process
SPISRC = armspi.c
SPISRC += spicrc.c
SPISRC += armutil.c
SPISRC += unipiutil.c
# SRC = $(SPISRC) nb_modbus.c virtual_regs.c
SRC = $(SPISRC) virtual_regs.c

BINFILES = fwspi fwserial unipihostname unipicheck unipimqtt

# Define optimisation level here
#OPT = -Ofast
#OPT = -Os

INCDIR  = $(patsubst %,-I%, $(INCDIRS))
LIBDIR  = $(patsubst %,-L%, $(LIBDIRS))
LIB     = $(patsubst %,-l%, $(LIBS))

OBJS  = $(SRC:.c=.o)
SPIOBJS  = $(SPISRC:.c=.o)

LDFLAGS = $(LIBDIR) $(LIB)

#
# makefile rules
#
all: $(BINFILES)

%.o: %.c
	$(CC) -c $(LDFLAGS) -std=c11 -I . $(INCDIR) $(PROJECT_VERSION) $< -o $@

.PHONY: unipimqtt fwspi fwserial

fwspi:  fwspi.o $(OBJS)
	$(CC) fwspi.o $(OBJS) $(LDFLAGS) -o $@
	@chmod +x $@

fwserial: fwserial.o armutil.o
	$(CC) $+ $(LDFLAGS) -o $@
	@chmod +x $@

unipihostname: unipihostname.o unipiutil.o
	$(CC) unipihostname.o unipiutil.o -o $@
	@chmod +x $@

unipicheck:  unipicheck.o unipiutil.o
	$(CC) unipicheck.o unipiutil.o -o $@
	@chmod +x $@

bandwidth-client: bandwidth-client.o
	$(CC) bandwidth-client.o $(LDFLAGS) -o $@

unipimqtt: unipimqtt.o $(OBJS)
	# $(CC) unipimqtt.o -o $@
	$(CC) unipimqtt.o $(OBJS) $(LDFLAGS) -o $@
	@chmod +x $@

install:
	$(INSTALL) -D $(BINFILES) -t $(DESTDIR)/opt/unipi/tools

clean:
	-rm -f *.o
	-rm -f $(BINFILES)
#	-rm -rf $(SRC:.c=.lst)
#	-rm -f ../bin/*

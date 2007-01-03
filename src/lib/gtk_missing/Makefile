###############################################################################
#
# File: Makefile (library)
#
# Author: Pete Nagy
#
###############################################################################

SYSTEM = ../../..
MKINCLDIR = $(SYSTEM)/resource/mkinclude$(DEF_MKINCL)

###############################################################################
# GENERIC DEFS - SHOULD NOT BE CHANGED FOR INDIVIDUAL FILES                   #
###############################################################################

#
# Include generic definitions for programs
#

include $(MKINCLDIR)/generic

###############################################################################
# PROGRAM SPECIFIC DEFS - CHANGE FOR INDIVIDUAL PROGRAMS                      #
###############################################################################

#
# Library name
#

  LIB = gtkmissing

#
# Default to shared library
#

  USESHARED = yes

#
# Source files managed by this makefile
#

  CSRCS = gtkmissing.c

#
# Header files managed by this makefile
#

  CHEADERS =

#
# Locations of needed header files
#

INCLUDES += -I$(SYSTEM)/resource
INCLUDES += $(EV_INCS)

#
# Libraries to use in library dependency order
# INSTLIBS depends on ARCHLIBS depends on POSTLIBS
#

LDFLAGS += $(EV_LIBSL)

INSTLIBS += $(EV_LIBS)

###############################################################################
# MAKEFILE OBJECTS AND TARGETS                                                #
###############################################################################

#
# Include generic definitions
#

include $(MKINCLDIR)/$(ARCHTARGETS)targets

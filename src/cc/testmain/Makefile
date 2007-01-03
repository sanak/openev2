###############################################################################
#
# File: Makefile (program)
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
# Executable name
#

  EXE_NAME = testmain

#
# Source files managed by this makefile
#

  CSRCS = testmain.c

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

  INSTLIBS += -lgv $(EV_LIBS)

###############################################################################
# MAKEFILE OBJECTS AND TARGETS                                                #
###############################################################################

#
# Include generic definitions
#

include $(MKINCLDIR)/$(ARCHTARGETS)targets

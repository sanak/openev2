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

  LIB = evtess

#
# Default to shared library
#

  USESHARED = yes

#
# Source files managed by this makefile
#

  CSRCS = dict.c \
          memalloc.c \
	  normal.c \
	  priorityq.c \
          sweep.c \
          tessmono.c \
          geom.c \
          mesh.c \
          render.c \
          tess.c

#
# Header files managed by this makefile
#

  CHEADERS = dict-list.h \
             dict.h \
             geom.h \
             memalloc.h \
             mesh.h \
             normal.h \
             priorityq-heap.h \
             priorityq-sort.h \
             priorityq.h \
             render.h \
             sweep.h \
             tess.h \
             tessmono.h


#            priorityq-heap.c \  - was part of CHEADERS..

###############################################################################
# MAKEFILE OBJECTS AND TARGETS                                                #
###############################################################################

#
# Include generic definitions
#

include $(MKINCLDIR)/$(ARCHTARGETS)targets

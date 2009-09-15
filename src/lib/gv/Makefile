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

  LIB = gv
  ULIB = GV

#
# Default to shared library
#

  USESHARED = yes

#
# Default to python module AND shared library (for testmain)
#

  USEPYMOD = yes

#
# Source files managed by this makefile
#

  CSRCS = crs.c \
          dbfopen.c \
          gextra.c \
          gvareatool.c \
          gvautopan.c \
          gvdata.c \
          gvlayer.c \
          gvlinetool.c \
          gvmanager.c \
          gvmarshal.c \
          gvmesh.c \
          gvnodetool.c \
          gvogr.c \
          gv_override.c \
          gvpointtool.c \
          gvpoitool.c \
          gvpquerylayer.c \
          gvprint.c \
          gvproperties.c \
          gvraster.c \
          gvrasteraverage.c \
          gvrastercache.c \
          gvrasterconvert.c \
          gvrasterize.c \
          gvrasterlayer.c \
          gvrasterlut.c \
          gvrastersource.c \
          gvrecttool.c \
          gvrenderinfo.c \
          gvroitool.c \
          gvrotatetool.c \
          gvselecttool.c \
          gvshape.c \
          gvshapefile.c \
          gvshapelayer.c \
          gvshapes.c \
          gvshapeslayer.c \
          gvskirt.c \
          gvsymbolmanager.c \
          gvtessshape.c \
          gvtexturecache.c \
          gvtool.c \
          gvtoolbox.c \
          gvtracktool.c \
          gvundo.c \
          gvutils.c \
          gvviewarea.c \
          gvviewlink.c \
          gvwinprint.c \
          gvzoompantool.c \
          invdistance.c \
          llrasterize.c \
          shpopen.c

#
# Header files managed by this makefile
#

  CHEADERS = crs.h \
             gextra.h \
             gv_config.h \
             gvareatool.h \
             gvautopan.h \
             gvdata.h \
             gview.h \
             gvlayer.h \
             gvlinetool.h \
             gvmanager.h \
             gvmarshal.h \
             gvmesh.h \
             gvnodetool.h \
             gvpointtool.h \
             gvpoitool.h \
             gvpquerylayer.h \
             gvproperties.h \
             gvraster.h \
             gvrasteraverage.h \
             gvrastercache.h \
             gvrasterize.h \
             gvrasterlayer.h \
             gvrasterlut.h \
             gvrastertypes.h \
             gvrecttool.h \
             gvrenderinfo.h \
             gvroitool.h \
             gvrotatetool.h \
             gvselecttool.h \
             gvshapelayer.h \
             gvshapes.h \
             gvshapeslayer.h \
             gvsymbolmanager.h \
             gvtool.h \
             gvtoolbox.h \
             gvtracktool.h \
             gvtypes.h \
             gvundo.h \
             gvutils.h \
             gvviewarea.h \
             gvviewlink.h \
             gvzoompantool.h \
             invdistance.h \
             gvshape.h
#
# Headers which when added to CHEADERS are all headers managed
# by this makefile.  These headers differ from CHEADERS in that
# they will not be used by python module automatic code generation
#

  LOCALINCS = shapefil.h gv_override.h

#
# Locations of needed header files
#

  INCLUDES += -I$(SYSTEM)/resource
  INCLUDES += $(EV_INCS) -I/usr/include/pygtk-2.0 `python-config --includes`
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

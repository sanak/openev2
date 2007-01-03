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

#
# Default to shared library
#

  USESHARED = yes

#
# Default to python module AND shared library (for testmain)
#

  USEPYMOD = both

#
# Source files managed by this makefile
#

  CSRCS = appcurlayer.c \
          crs.c \
          dbfopen.c \
          gextra.c \
          gtkcolorwell.c \
          gvarealayer.c \
          gvareas.c \
          gvareatool.c \
          gvdata.c \
          gvlayer.c \
          gvlinelayer.c \
          gvlinetool.c \
          gvmanager.c \
          gvmarshal.c \
          gvmesh.c \
          gvnodetool.c \
          gvogr.c \
	  gv_override.c \
          gvpointlayer.c \
          gvpoints.c \
          gvpointtool.c \
          gvpoitool.c \
          gvpolylines.c \
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
          gvrecords.c \
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
          gvtess.c \
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
          ipgcplayer.c \
          llrasterize.c \
          shpopen.c

#
# Header files managed by this makefile
#

  CHEADERS = appcurlayer.h \
             crs.h \
             dllfct.h \
             gextra.h \
             glu.h \
             gluos.h \
             gtkcolorwell.h \
             gv_config.h \
             gvarealayer.h \
             gvareas.h \
             gvareatool.h \
             gvdata.h \
             gview.h \
             gvlayer.h \
             gvlinelayer.h \
             gvlinetool.h \
             gvmanager.h \
             gvmarshal.h \
             gvmesh.h \
             gvmodule.h \
             gvnodetool.h \
             gvpointlayer.h \
             gvpoints.h \
             gvpointtool.h \
             gvpoitool.h \
             gvpolylines.h \
             gvpquerylayer.h \
             gvproperties.h \
             gvraster.h \
             gvrasteraverage.h \
             gvrastercache.h \
             gvrasterize.h \
             gvrasterlayer.h \
             gvrasterlut.h \
             gvrastertypes.h \
             gvrecords.h \
             gvrecttool.h \
             gvrenderinfo.h \
             gvroitool.h \
             gvrotatetool.h \
             gvselecttool.h \
             gvshapelayer.h \
             gvshapes.h \
             gvshapeslayer.h \
             gvsymbolmanager.h \
             gvtess.h \
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
             ipgcplayer.h \
		 gvshape.h
#
# Headers which when added to CHEADERS are all headers managed
# by this makefile.  These headers differ from CHEADERS in that
# they will not be used by python module automatic code generation
#

  LOCALINCS = glu.h gluos.h shapefil.h gv_override.h

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

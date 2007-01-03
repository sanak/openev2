################################################################################
#
# Module: pymod Makefile
#
# Purpose to manage pymod build
#
# Author: Pete Nagy
#
################################################################################

SYSTEM = ../..
MKINCLDIR = $(SYSTEM)/resource/mkinclude$(DEF_MKINCL)

################################################################################
# GENERIC DEFS - SHOULD NOT BE CHANGED FOR INDIVIDUAL FILES                    #
################################################################################

#
# Include generic definitions
#
include $(MKINCLDIR)/generic

#
# Garbage files to move to .cvsignore'd junk directory
#

  GARBAGE = *.dll *.shp *.dbf *.shx *.pbm *.txt

#
# Description 
#

what:
	@ echo ""; \
	echo "-----------------------------------------------------"; \
	echo "Clean up or prepare extraneous stuff"; \
	echo "Targets: clean, junk"; \
	echo "-----------------------------------------------------"; \

all:
install:

#
# Clean up pyc, dlls, shapefiles in pymod dir
#

clean:
	@ $(RM) *.pyc *~ .*~ .#*#
	@ $(MKDIR) junk
	@ for filename in $(GARBAGE) dummyfile ; do \
	  if [ -f $$filename ]; then \
	    $(MV) $$filename junk; \
	  fi; \
	done
	@ echo "Directory clean; dlls and data moved to junk"

#
# Move stuff from junk to local directory (needs second target for some reason)
#

junk: junk1
junk1:
	$(MV) junk/* .

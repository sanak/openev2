################################################################################
#
# Module:  src Makefile
#
# Purpose: To manage project resource build
#
# Targets: delivery
#
# Author:  Pete Nagy
# Version: %I%, %G%
#
#1234567890123456789012345678901234567890123456789012345678901234567890123456789
################################################################################

SYSTEM = ..
MKINCLDIR = $(SYSTEM)/resource/mkinclude$(DEF_MKINCL)

DLV_DIR = $(SYSTEM)/output/delivery

#
# Targets:
#

info: build_tag
	@ echo Targets: delivery
	@ echo

build_tag:
	@ echo
	@ echo "=================="
	@ echo "  RESOURCE BUILD  "
	@ echo "=================="
	@ echo

delivery_build_tag:
	@ echo
	@ echo "=============================="
	@ echo "  CREATING DELIVERY LOCATION  "
	@ echo "=============================="
	@ echo

#
# Create 'delivery' directory by copying needed built files to the
# output/delivery directory for the current platform.
#

delivery: $(ARCH)_delivery

CYGWIN_NT-5.1_delivery: delivery_build_tag
	@ echo Copying to $(DLV_DIR)
	@ $(MKDIR)$(DLV_DIR)
	@ $(CP) `pkg-config 

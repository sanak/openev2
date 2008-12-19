###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  DEPRECATED: now only subclassing new pgu.ComboBoxEntry
# Author:   Mario Beauchamp (starged@gmail.com)
#
###############################################################################
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
###############################################################################

from pgu import ComboBoxEntry

class pguCombo(ComboBoxEntry):
    """
    this class subclasses ComboBoxEntry for backwards compatibility.
    """
    def __init__(self, strings=[""]):
        ComboBoxEntry.__init__(self, strings)
        self.items = strings
        self.current_item = 0

    def get_selected_item(self):
        """Return the index of the selected item
        """
        return self.get_active()
        
    def select_item(self, item=0):
        """Select an item by it's index
        """
        try:
            self.set_active(item)
            self.current_item = item
            return True
        except IndexError:
            self.set_active(-1)
            self.current_item = -1
            return False

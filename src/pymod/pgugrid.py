#! /usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  General purpose grid widget.
# Author:   Gillian Walter, gillian.walter@atlantis-scientific.com
#
###############################################################################
# Copyright (c) 2003, Atlantis Scientific Inc. (www.atlantis-scientific.com)
#
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
# To do soon:
# - Simplify default window startup
#
#
# To do later (maybe):
# - Ability to dump entire or visible grid to printer or file
# - column resizing
# - row and column title user configuration
# - per-row editability options (some rows deletable/changeable, some not)
# - data input checking (may need to be externally influenced)
# - add up/down/left/right arrow actions.

# NOTE: This grid has to simulate double click behaviour by tracking the
# last clicked row/column/cell because on windows a) the gtk double click
# event is not sent out if the callback on the single click event is too
# long (this can happen for row selection when there are a large number
# of shapes- on the order of 50000- and the expose event demands a lot
# of the cpu),
# and b) even if the double click event is sent out, the associated
# single press events are sent out in different orders on windows than
# they are on linux/unix.  The sequence of events sent out for a double
# click is as follows:
# linux/unix: BUTTON_PRESS, BUTTON_PRESS, _2BUTTON_PRESS
# windows: BUTTON_PRESS, BUTTON_PRESS (if single press callback is too long)
#          BUTTON_PRESS, _2BUTTON_PRESS, BUTTON_PRESS
#
# GTK's double click behaviour is reasonable for linux/unix, but for
# consistency the approach for all platforms has been to treat the
# double click event as a single click event and track the last
# clicked cell/row/column internally.  Internally, the grid only uses the
# simulated double click event to launch cell editing.
#
# The better alternative would be to fix gtk itself for windows, but this
# could be complicated and would require new users to build a special
# version of gtk for windows...
#
# THIS SHOULD BE REVISITED IF OPENEV IS UPGRADED TO GTK2, AS GTK2 MAY
# NOT SUFFER THESE PROBLEMS.

import sys
import traceback
import pygtk
pygtk.require('2.0')
import gtk
from gtk.keysyms import *
from gtk.gdk import *
from gvsignaler import Signaler
import numpy as Numeric
import gview
import gvutils


# Allowable data sources for pgugrid
SRC_NONE=0 # no source
SRC_NUMERIC=1 # 2 dimensional python array
SRC_SHAPES=2  # Shapes
SRC_SHAPESLAYER=3 # Shapes layer (layer selection follows grid's)
SRC_LISTLIST=4 # List of lists
SRC_LISTOBJ=5 # List of objects
SRC_LISTUNDEF=6 # Empty list (will be updated with either lists or objects)

# Internal supporting classes
class _column_info:
    """ Class to manage display-related information for
        a single column in the pgugrid.

        Members:

          member- Reference to the source being displayed:
                  SRC_NUMERIC- data column
                  SRC_SHAPES- property name
                  SRC_SHAPESLAYER- property name
                  SRC_LISTLIST- index within sub list
                  SRC_LISTOBJ- variable name within object

          title- Column display title.

          type- type of value: 'string','integer','float', or 'complex'.

          format- a string to indicate how values should be
                  displayed in the column.  Format string also
                  determines text justification (left, right,
                  or center) within the cell

          editable- editable status of the column.  Indicates
                    whether or not the values in the column
                    can be changed other than row addition/
                    deletion.
                    0- not editable
                    1- editable

          Note: The editability of a given cell is determined by
                the intersection of the column and row editability
                parameters.  Add/delete is determined entirely
                by the row add/delete settings.  The contents of
                the cell can only be altered (aside from add/delete)
                if both the row and the column permit change.
                Editable status of rows can currently only be set
                on the grid as a whole, not per-row (though
                this may be added later).  Row editability
                follows these rules:
                    0- not editable (000)
                    1- rows can be deleted, but not added/changed (001)
                    2- rows can be added, but not deleted/changed (010)
                    4- rows can be changed, but not deleted/added (100)
                    3- rows can be deleted/added, but not changed (011)
                    5- rows can be changed or deleted, but not added (101)
                    6- rows can be changed or added, but not deleted (110)
                    7- rows can be changed and added/deleted (111)        


          nodata- string to display if no data is available

          width- width of the column

          start_x- x location of top left corner of column header

          justification- text justification in the column: 0
                         for right, 1 for left, 2 for center.

          title_justification- text justification of column title: 0
                         for right, 1 for left, 2 for center.

          force_width- force a column to be a particular width.

          force_width_chars- force a column to be a particular number
                             of characters wide.  If active, will overwrite
                             force_width with force_width_chars * maximum
                             character width for the current font.  Set
                             to -1 to inactivate, or an integer >= 0
                             corresponding to number of characters
                             to allow to activate.  Defaults to -1.

          entry_chars- number of characters to allow user to type
                       (only relevant if column is editable).  90
                       by default.




    """
    def __init__(self,member,title,type,format=None,editable=0,
                 nodata='',width=0,start_x=0,justification=0,
                 title_justification=2,force_width=None,
                 force_width_chars=-1, entry_chars=0):

        self.title=title
        self.member=member
        self.type=type
        self.format=format
        self.editable=editable
        self.nodata=nodata
        self.width=width
        self.start_x=start_x
        self.justification=justification
        self.title_justification=title_justification
        # force_width is used to limit a column's width 
        # width for display (set to None to auto-choose).
        self.force_width=force_width
        self.force_width_chars=force_width_chars
        self.entry_chars=entry_chars



# Configurable events that grid can respond to


# Cell area has been clicked on
_cell_clickevents=('cell-left','cell-shift-left','cell-ctrl-left',\
                   'cell-double-left','cell-right',\
                   'cell-shift-right','cell-ctrl-right',\
                   'cell-double-right')

# Row title has been clicked on
_row_clickevents=('row-left','row-shift-left','row-ctrl-left',\
                  'row-double-left','row-right','row-shift-right',\
                  'row-ctrl-right','row-double-right')

# Column title has been clicked on
_column_clickevents=('column-left','column-shift-left','column-ctrl-left',\
                  'column-double-left','column-right','column-shift-right',\
                  'column-ctrl-right','column-double-right')

_cell_keyevents=()


_row_select=[(),\
           ('select-single-row','unselect-single-row','toggle-single-row'),\
           ('select-single-row','unselect-single-row','toggle-single-row',\
            'toggle-block-rows','toggle-multiple-rows','select-block-rows',
            'select-multiple-rows')]

_cell_select=[(),\
                  ('select-single-cell','unselect-single-cell',\
                   'toggle-single-cell'),\
                  ('select-single-cell','unselect-single-cell',\
                   'toggle-single-cell','toggle-block-cells',\
                   'toggle-multiple-cells')]

_column_select=[(),
                 ('select-single-column','unselect-single-column',\
                  'toggle-single-column'),\
                 ('select-single-column','unselect-single-column',\
                  'toggle-single-column','toggle-block-columns',\
                  'toggle-multiple-columns','select-block-columns',
                  'select-multiple-columns')]


class _selection_info:
    """Selection behaviour parameters:

    Several parameters must be defined in order to indicate the selection
    behaviour within the grid.  Here, row selection refers to the selection
    of an entire row or rows, column selection refers to the selection of
    an entire column or columns, and cell selection refers to the
    selection of a single or multiple individual cells.  The possible
    values of each are 0 (no selection), 1 (single selection), or
    2 (multiple selection).  The combination parameters indicate whether
    or not different types of selection can be present at the same time
    For instance, setting the row-cell selection
    parameter to 2 indicates that the last selected row can remain
    selected while a cell within that row or another row is selected.
    Setting it to 1 indicates that the last selected row can only remain
    selected if the cell just selected is within that row.  Setting it
    to 0 indicates that the row will be unselected when a cell is 
    selected.  Setting it to 3 indicates that selecting a cell should
    force selection of that row if it is not already selected.
    For row-column selection, the two are orthogonal, so only
    0 (rows/columns can't be selected at the same time) and
    1 (rows/columns can be selected at the same time) are available.

    Note: a cell, row, or column must be selected in order to be
          changed or deleted through the pgugrid.

    1) row selection- 0, 1, or 2
    2) column selection- 0, 1, or 2
    3) cell selection- 0, 1, or 2
    4) row-cell selection- 0, 1, 2, 3
    5) column-cell selection- 0, 1, 2, 3
    6) row-column selection- 0 or 1

    Currently, this class just keeps track of which selection
    actions should be permitted for a given selection configuration
    (verified through the get_allowed_actions function).
    """  

    def __init__(self,row_selection=1,column_selection=1,cell_selection=0,
               row_cell=0, column_cell=0, row_column=0):

        if row_selection not in [0,1,2]:
            raise AttributeError,'row_selection must be 0, 1, or 2'

        if cell_selection not in [0,1,2]:
            raise AttributeError,'cell_selection must be 0, 1, or 2'

        if column_selection not in [0,1,2]:
            raise AttributeError,'column_selection must be 0, 1, or 2'

        if row_cell not in [0,1,2,3]:
            raise AttributeError,'row_cell must be 0, 1, 2, or 3'

        if column_cell not in [0,1,2,3]:
            raise AttributeError,'column_cell must be 0, 1, 2, or 3'

        if row_column not in [0,1]:
            raise AttributeError,'row_column must be 0 or 1'

        self.row=row_selection
        self.cell=cell_selection
        self.column=column_selection
        self.row_cell=row_cell
        self.column_cell=column_cell
        self.row_column=row_column

    def get_allowed_actions(self):
        select_actions=[]
        for item in _row_select[self.row]:
            select_actions.append(item)

        for item in _cell_select[self.cell]:
            select_actions.append(item)

        for item in _column_select[self.column]:
            select_actions.append(item)

        return select_actions


class _pgugrid_options:

    """Class to set up pgugrid configuration.
    Stores whether or not row and column titles will be shown,
    and configures event mapping within the grid.
    Input parameter is a tuple of 9 values indicating configuration
    information.

    NOTE 1: These parameters control selection through clicking on the grid.
            In cases where selection can be done from outside (eg.
            shapes layer through selection tool, or by directly calling
            a grid function), it is the responsibility of the calling
            application to ensure that selection consistency is maintained.
            For instance, if row selection is single, the layer's
            selection mode (for select tool) should also be set to
            single; otherwise the grid may end up with multiple
            selected rows (it stays consistent with the layer selection
            regardless of parameter settings).

    NOTE 2: NOT ALL CONFIGURATIONS HAVE BEEN IMPLEMENTED/TESTED YET,
            AND SOME MAY NOT BE IMPLEMENTED EVER.

    SUPPORTED/VALID CONFIGURATIONS:

    Parameter: 0   1   2   3   4   5   6   7 

               0 - 0 - 0 - 0 - 0 - 0 - 0 - 0
                           1   4
                           2

               1 - 0 - 0 - 0 - 0 - 0 - 0 - 0 
                   1       1   4   1          
                   2       2       2

               2 - 0 - 0 - 0 - 0 - 0 - 0 - 0 
                       1   1   4               
                       2   2

               3 - 0 - 0 - 0 - 0 - 0 - 0 - 0 
                   1   1   1   4   1           
                   2   2   2       2


               0 - 1 - 0 - 1 - 0 - 3 - 0 - 0
               1               4   
               2          
               3

               0 - 2 - 0 - 2 - 0 - 3 - 0 - 0
               1               4   
               2          
               3


    Parameters:

    [0]: Row/Column titles

         0- no row or column titles
         1- row titles only
         2- column titles only
         3- row and column titles

    [1]: Row selection mode

         0- rows may not be selected

         If row titles are present:
             1- rows may be singly selected
             2- multiple rows may be selected

    [2]: Column selection mode

         0- columns may not be selected

         If column titles are present:
             1- columns may be singly selected
             2- multiple columns may be selected

    [3]: Cell selection mode

         0- cells may not be selected
         1- cells may be singly selected
         2- cells may be multiply selected

    [4]: Row editability

         0- rows are not editable (000)
         1- rows can be deleted, but not added/changed (001)
         2- rows can be added, but not deleted/changed (010)
         4- rows can be changed, but not deleted/added (100)
         3- rows can be deleted/added, but not changed (011)
         5- rows can be changed or deleted, but not added (101)
         6- rows can be changed or added, but not deleted (110)
         7- rows can be changed and added/deleted (111)        

         Note: column editability is set by the user using define_columns
         (if user doesn't set it, defaults are used).


    [5]: Row/Cell cross selection behaviour

         0- rows and cells may not remain simultaneously selected
         1- rows and cells can remain selected if the selected cell
            is in the selected row
         2- rows and cells can remain selected simultaneously even
            if the cell isn't in the row
         3- selecting a cell forces selection of that row, clearing
            rows that do not contain selected cells, and vice versa
            (row selection is synchronized with cell selection).
            NOTE: IN THIS MODE, LEFT CLICKING A CELL TOGGLES ITS
            SELECTION STATE RATHER THAN ALWAYS FORCING IT TO BE
            SELECTED IF THE EDITING PARAMETER IS TURNED OFF.

    [6]: Column/Cell cross selection behaviour

         0- columns and cells may not remain simultaneously selected
         1- columns and cells can remain selected if the selected cell
            is in the selected column
         2- columns and cells can remain selected simultaneously even
            if the cell isn't in the column
         3- selecting a cell forces selection of that column, clearing
            columns that do not contain selected cells.  Note:
            in this case cell selection parameter is ignored (when
            a column is selected, all cells in that column are selected).

    [7]: Row/Column cross selection behaviour

         0- rows and columns may not remain simultaneously selected
         1- rows and columns may be simultaneously selected

    [8]  Variant parameter. Used to do slight variations on the above
         configuration parameters (eg. turning column sorting on/off,
         alterations to which events trigger selection/editing etc.,
         turning off grid lines).  ALL variants must be documented here:

         0 or None: No variation
         1,3,...: No column sorting (note: some configurations don't anyway)
         2,3,5,6,..: For configurations where cells are editable and cell
              selection does not trigger row selection, have cell editing
              trigger on single left click rather than double click.
              IE.  This only has an effect if config[5] is not 3, and
              config[4] is one of 4,5,6, or 7.
         4,5,...: For configurations where row and/or column selection
              is enabled, unselection using a second click is disabled
              (it just triggers selection again).

         Variant parameter= (column sorting off)*1 +
                            (start editing on single left click)*2 +
                            (second click unselects)*4

    Input parameters: configuration

    Valid options for configuration:

       - click events:

           Row selection (if enabled): NOTE: REQUIRES THAT ROW TITLES
                                             BE PRESENT!
               single left click on a row title- toggle row's selection

               shift left click on row title-
                   multiple consecutive row selection

               control left click on row title-
                   multiple nonconsecutive row selection

               control right click on row title- If the source is a
                   shapes layer, translate the view to that row and
                   add that row to the current selection if possible
                   (clear other selections if only single selection
                   is enabled).

           Cell selection (if enabled):
               if self.config[5] is 3:

                   control right click on cell- If the source is a
                       shapes layer, translate the view to that row and
                       add that row to the current selection if possible
                       (clear other selections if only single selection
                       is enabled).

               if self.config[5] is not 3 or self.config[4] is in [4,5,6,7]:

               single left click a cell- select a cell.  Press escape or
                                          select another cell to unselect.

               if self.config[5] is 3 and self.config[4] is not in
               [4,5,6,7]:
                   single left click a cell- toggle a cell's selection.


           Column selection (if enabled): NOTE: REQUIRES THAT COLUMN TITLES
                                                BE PRESENT!

               single left click on column title- toggle column's selection

               shift left click on column title-
                   multiple consecutive column selection

               control left click on column title-
                   multiple nonconsecutive column selection               

           Cell editing (if enabled):

               left clicking a cell twice in a row-
                   Start cell editing.
                   Pressing Enter or Tab, or selecting
                   another cell will finish editing
                   and store the changes.  Enter will
                   finish by unselecting the cell
                   and selecting the next one down
                   in the column for editting.
                   Tab will finish by unselecting the
                   cell and selecting the one in the
                   next column for editting.  Pressing
                   Escape will exit editing mode, clear
                   the cell selection, and cancel the
                   changes.

           Other behaviours:
               alt left click on table- if the source is SRC_SHAPESLAYER,
                                        recenter the view on the selected
                                        shape.

               single right click on column title- sort by the values in
                                                      that column (reverse at
                                                      each click)


               arrow keys: (NOT IMPLEMENTED YET)
               up/down:
               - if a single row is selected, move the selected row up/down
               - if a cell is selected, unselect the cell, move up/down,
                 and select the next cell.  If the cell is being edited,
                 store the changes before unselecting and moving on, and
                 select the next cell for editing as well.

               right/left:
               - if a cell is selected, unselect the cell, move right/left,
                 and select the next cell.
               - if a cell is selected and being edited, move right/left
                 within the entry.

           Note on row-cell selection behaviour (column-cell is analogous):

           row-cell        row selected             cell selected
           parameter

              0          unselect all cells,        unselect all rows
                         cancel cell editing

              1          unselect all cells        unselect all rows that
                         not in a selected         don't contain a
                         row, cancel cell          selected cell
                         editing if edited cell
                         is unselected

              2          no effect on cells        no effect on rows

              3          select first cell in      add cell's row to selected
                         the row if no cell in            row list
                         that row is selected yet


           row-cell        row unselected             cell unselected
           parameter

              0          no effect on cells        no effect on rows

              1          if # rows is still        if # cells is still 
                         > 0, unselect any         > 0, unselect any
                         cells in the unselected   rows that don't
                         row and cancel cell       contain a selected
                         editing of unselected     cell
                         cells

              2          no effect on cells        no effect on rows

              3          unselect all selected     unselect that row
                         cells in that row         if no selected cells
                                                   remain in the row


    """

    def __init__(self,configuration=None):

        # Set defaults where user has not specified anything
        cfg_defaults=(2,2,0,0,0,2,0,0,0)
        if configuration is None:
            configuration=cfg_defaults
        else:
            cfg=[]
            for idx in range(9):
                if len(configuration) <= idx:
                    cfg.append(cfg_defaults[idx])
                elif configuration[idx] is None:
                    cfg.append(cfg_defaults[idx])
                else:
                    cfg.append(configuration[idx])

            configuration=tuple(cfg)

        self.config=configuration

        # Check configuration
        _ranges=[]
        _ranges.append([0,1,2,3])
        _ranges.append([0,1,2])
        _ranges.append([0,1,2])
        _ranges.append([0,1,2])
        _ranges.append([0,1,2,3,4,5,6,7])
        _ranges.append([0,1,2,3])
        _ranges.append([0,1,2,3])
        _ranges.append([0,1])
        _names=['display','row select','column select','cell select','edit',
                'row-cell cross selection','column-cell cross selection',
                'row-column cross selection']

        _invalid=[(0,None,1,None,None,None,None,None),
                  (0,None,2,None,None,None,None,None)]

        for idx in range(len(_ranges)):
            if self.config[idx] not in _ranges[idx]:
                txt="_pgugrid_options: invalid configuration specification"+\
                    ".\nValid ranges are (0-3,0-2,0-2,0-2,0-7,0-3,0-3,0-1)."
                raise AttributeError,txt

        for cfgtype in _invalid:
            matches=0
            idx=0
            for item in cfgtype:
                if item is None:
                    matches=matches+1
                elif item == self.config[idx]:
                    matches=matches+1
                idx=idx+1
            if matches == 8:
                txt="_pgugrid_options: invalid configuration combination.\n"
                idx=0
                for item in cfgtype:
                    if item is not None:
                        txt=txt+_names[idx]+": "+str(self.config[idx])+"\n"
                        raise AttributeError,txt

        self.events={}
        self._allowed_events=[]

        for item in _cell_clickevents:
            self._allowed_events.append(item)

        for item in _cell_keyevents:
            self._allowed_events.append(item)

        # row title type: only used if row titles are shown
        # Should be 'grid' to show grid row, 'source' to show
        # underlying source index.

        self.row_title_type = 'grid'

        if self.config[0] in [1,3]:
            self.show_row_titles=1
            for item in _row_clickevents:
                self._allowed_events.append(item)
        else:
            self.show_row_titles=0

        if self.config[0] in [2,3]:
            self.show_column_titles=1
            for item in _column_clickevents:
                self._allowed_events.append(item)
        else:
            self.show_column_titles=0

        self._set_selection_info()
        self._set_event_mapping()

    def set_row_title_type(self,type):
        """Define whether row titles should reflect grid
           row number, or underlying source index.
           Parameters:
               type- either 'grid' (to show grid row) or
                     'source' (to show source row index).
        """
        if type not in ['grid','source']:
            raise AttributeError,"_pgugrid_options: row title type must be "+\
                                 "either 'grid' or 'source'"

        self.row_title_type = type

    def _set_selection_info(self):
        s1=self.config[1]
        s2=self.config[2]
        s3=self.config[3]
        s4=self.config[5]
        s5=self.config[6]
        s6=self.config[7]
        self.selection_info=_selection_info(s1,s2,s3,s4,s5,s6)

    def _extract_variant_options(self):
        """ Extract the variant parameters and return a tuple
            of which ones are turned on (0 if they're off,
            1 if they're on:
            (column sorting, editing starts on single left click,
            second click unselects)
        """
        leftover = self.config[8]
        if (leftover >= 4):
            par2 = 1
            leftover = leftover - 4
        else:
            par2 = 0

        if (leftover >= 2):
            par1 = 1
            leftover = leftover - 2
        else:
            par1 = 0

        par0 = leftover

        return (par0, par1, par2)

    def _set_event_mapping(self):
        """Map events onto behaviours."""

        # NOTE: the setting of some events to both double and single
        # clicks is because of the hacky way double clicks are detected
        # here (two consecutive clicks on the same thing any time
        # apart) because of problems on windows.  These ensure that
        # single click events behave okay, but still allows user have
        # a pseudo-double click event.  NEEDS TO BE FIXED UP!!!
        vopts = self._extract_variant_options()
        # row selection
        if self.show_row_titles == 1:
            if self.selection_info.row > 0:
                if vopts[2] == 1:
                    self.events['row-left']='select-single-row'
                    self.events['row-double-left']='select-single-row'
                else:
                    self.events['row-left']='toggle-single-row'
                    self.events['row-double-left']='toggle-single-row'
                self.events['row-ctrl-right']='translate-view-to-row'
            if self.selection_info.row > 1:
                if vopts[2] == 1:
                    self.events['row-shift-left']='select-block-rows'
                    self.events['row-ctrl-left']='select-multiple-rows'
                else:
                    self.events['row-shift-left']='toggle-block-rows'
                    self.events['row-ctrl-left']='toggle-multiple-rows'

        # column sorting and selection
        if ((self.show_column_titles == 1) and
            (vopts[0] == 0)):
            self.events['column-right']='toggle-sort-by-column'
            self.events['column-double-right']='toggle-sort-by-column'

            if self.selection_info.column > 0:
                if vopts[2] == 1:
                    self.events['column-left']='select-single-column'
                    self.events['column-double-left']='select-single-column'
                else:
                    self.events['column-left']='toggle-single-column'
                    self.events['column-double-left']='toggle-single-column'

            if self.selection_info.column > 1:
                if vopts[2] == 1:
                    self.events['column-shift-left']='select-block-columns'
                    self.events['column-ctrl-left']='select-multiple-columns'
                else:
                    self.events['column-shift-left']='toggle-block-columns'
                    self.events['column-ctrl-left']='toggle-multiple-columns'

        # cell selection and editing
        if self.selection_info.cell > 0:
            if self.config[5] == 3:
                if self.config[4] in [4,5,6,7]:
                    self.events['cell-left']='select-single-cell'
                    self.events['cell-double-left']='start-cell-edit'
                else:
                    if vopts[2] == 1:
                        self.events['cell-left']='select-single-cell'
                        self.events['cell-double-left']='select-single-cell'
                    else:
                        self.events['cell-left']='toggle-single-cell'
                        self.events['cell-double-left']='toggle-single-cell'
                self.events['cell-ctrl-right']='translate-view-to-row'
            else:
                if vopts[1] == 1:
                    self.events['cell-left']='select-single-cell'
                    if self.config[4] in [4,5,6,7]:
                        # Note: cell editing function needs to check that
                        # column is editable before altering anything.
                        self.events['cell-double-left']='start-cell-edit'
                else:
                    if self.config[4] in [4,5,6,7]:
                        # Note: cell editing function needs to check that
                        # column is editable before altering anything.
                        self.events['cell-left']='start-cell-edit'

            if self.selection_info.cell > 1:
                self.events['cell-shift-left']='toggle-block-cells'
                self.events['cell-ctrl-left']='toggle-multiple-cells'


        # The remaining code in this function is used to
        # validate a new configuration.
        # It can be commented out for run-time.

        # get allowable selection actions
        allowed=self.selection_info.get_allowed_actions()

        # sorting columns is always permitted
        allowed.extend(['sort-by-column','reverse-sort-by-column',
                        'toggle-sort-by-column'])

        allowed.extend(['translate-view-to-row'])

        # edit-related actions
        if self.config[4] in [4,5,6,7]:
            allowed.extend(['start-cell-edit'])

        # check that this configuration does not define
        # invalid actions (eg. editing in non-editable grid)
        for action in self.events.values():
            if action not in allowed:
                raise 'Invalid action mapping in pgugrid configuration'

        # check that configuration doesn't define callbacks
        # for events that won't happen
        for cevent in self.events.keys():
            if cevent not in self._allowed_events:
                raise 'Invalid event in pgugrid configuration'

# Grid
class pguGrid(gtk.HBox,Signaler):
    def __init__(self,config):
        """ Class to create a tabular display grid.

            Parameters:
                config- configuration to use (controls
                        whether row and column titles
                        are displayed, and how rows
                        and columns can be selected).
                        See _pgugrid_options class
                        documentation for more.
        """

        # both super classes have notify methods which conflict
        # we need the gvsignaler's one
        #gtk.Table.__init__(self,rows=2,cols=3)
        gtk.HBox.__init__(self)
        self.set_spacing(0)
        self.vshell = gtk.VBox()
        self.vshell.set_spacing(0)
        self.pack_start(self.vshell)


        self.opts=_pgugrid_options(config)
        self._ColumnDefs=[]

        # GUI setup

        # default style
        self.default_style = None
        self.default_style_reset_flag = 0
        self.default_row_title_style = None
        self.default_row_title_style_reset_flag = 0
        self.default_col_title_style = None
        self.default_col_title_style_reset_flag = 0

        self.draw_row_lines = 2
        self.draw_col_lines = 2

        #fonts for drawing titles and cells put here
        self.title_font = None
        self.cell_font = None

        # Message to display when there is no data
        self.empty_msg= "NO DATA TO DISPLAY"

        # The height of a single row (not including the
        # line between rows) and column title row (if present),
        # width of row titles column (if present).  These
        # get reset in configure.
        self.row_height = 0
        self.column_title_height=0
        self.row_title_width = 0

        # padding between lines and text in cells
        # (included in column width/row heights)
        self.pad=4 

        # Build GUI
        self.hadj = gtk.Adjustment()
        self.vadj = gtk.Adjustment()

        self._hscroll = gtk.HScrollbar(self.hadj)
        self._vscroll = gtk.VScrollbar(self.vadj)
        self.hsframe = gtk.Frame()
        self.hsframe.set_shadow_type(gtk.SHADOW_NONE)
        self.vsframe = gtk.Frame()
        self.vsframe.set_shadow_type(gtk.SHADOW_NONE)
        self.hsframe.add(self._hscroll)
        self.vsframe.add(self._vscroll)
        self.hscroll_shown=1
        self.vscroll_shown=1
        self.hscroll_policy = 0
        self.vscroll_policy = 0

        self._area = gtk.DrawingArea()
        self._pixmap = None

        #this mask also seems to enable scrolling???
        evt_mask = gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK | \
                   gtk.gdk.KEY_RELEASE_MASK | \
                   gtk.gdk.FOCUS_CHANGE_MASK | gtk.gdk.EXPOSURE_MASK 
        self._area.set_events( evt_mask )
        self._area.set_flags( gtk.CAN_FOCUS | gtk.HAS_GRAB )

        #flag to recalculate the adjustments
        self.bCalcAdjustments = True

        #set to true if changing some value that would end up causing multiple
        #expose events or an endless loop even.
        self.updating = False

        #frm = gtk.Frame()
        #frm.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        #frm.add(self._area)
        #self._area.set_size_request(300,400)

        self._layout=gtk.Layout()
        self._layout.put(self._area,0,0)
        self._layout.show_all()

        #self.attach( frm, 0, 1, 0, 1,
        #                    xoptions=gtk.FILL, yoptions=gtk.FILL )
        self.vshell.pack_start(self._layout,expand=True)
        self.vshell.pack_start(self.hsframe,expand=False)
        self.pack_start(self.vsframe,expand=False)
        #self.attach( self._layout, 0, 1, 0, 1,
        #                    xoptions=gtk.FILL, yoptions=gtk.FILL )
        #self.attach( self._vscroll, 1, 2, 0, 1, xoptions=gtk.SHRINK)
        #self.attach( self._hscroll, 0, 1, 1, 2, yoptions=gtk.SHRINK )

        # floating editing entry box 
        self.editbox = gtk.Entry()
        self.editbox.connect( 'key_press_event', self.entry_key_press )
        self.editbox.hide()

        self.editing_cell=None


        dw=300
        dh=400
        self.set_size_request(dw,dh)

        #self._layout=gtk.Layout()
        self._layout.put(self.editbox,5000,5000)        
        #self.add(self._layout)
        self.show_all()

        # Style list: used to colour rows.  Initially
        # empty except for default style placeholders
        # (filled in upon first expose).
        self.style_list=[None,None,None]

        # new styles can't actually be added until expose.
        # Use this to store info up until then
        self.styles_to_add=None

        # Initialize parameters relating to source displayed in grid
        self._initialize_settings()

        # signals: Note that the right-click (button 3) event
        # is a special case used internally to select cells for
        # editing.
        self.publish('row-selection-changed')
        #self.publish('row-changed')
        #self.publish('row-added')
        self.publish('rows-deleted')
        self.publish('cell-selection-changed')
        self.publish('cell-changed')
        self.publish('column-selection-changed')
        #self.publish('column-changed')
        #self.publish('column-added')
        #self.publish('column-deleted')

        # Pass on click events with grid row and column
        # (-1 if title clicked)
        self.publish('clicked')


        self._area.connect( 'expose-event', self.expose )
        self._area.connect( 'configure-event', self.configure )
        self._area.connect( 'button-press-event', self.click )
        self._area.connect( 'key_press_event', self.area_key_press )
        self.hadj.connect( 'value-changed', self.changed )
        self.vadj.connect( 'value-changed', self.changed )
        self.connect( 'style-set', self.expose )
        self.connect('size-allocate',self.size_allocate)

    def reset_configuration(self,config):
        """ Reset the grid configuration. """
        self.opts=_pgugrid_options(config)

        if self.opts.show_column_titles == 1:
            if self.title_font is not None:
                self.column_title_height = self.title_font.ascent + 2*self.pad
            else:
                self.row_title_width=0 
        else:
            self.column_title_height = 0

        if self.opts.show_row_titles == 1:
            if (self.title_font is not None) and (self.src is not None):
                self.row_title_width=self.title_font.string_width(
                    str(len(self.src2row)))+2*self.pad
            else:
                self.row_title_width=0
        else:
            self.row_title_width=0

        self.expose()


    def set_empty_message(self,msg):
        """ Set the message to display when there is no data. """
        self.empty_msg=msg

    def set_source(self,source,view=None,subset=None,members=None,titles=None,
                   editables=None,formats=None,types=None,nodata=None,
                   justify=None,title_justify=None,force_width=None,
                   force_width_chars=None, entry_chars=None, expose=0,
                   redefine_columns=1):
        """ Set the source for the grid.
            Note: for Numeric arrays, the
            grid operates on a copy of the
            source data.  For all other
            types, it operates on the
            original.

            Parameters:
                source- one of:
                        a) 1 or 2-D Numeric python array
                        b) List of same-length lists
                        c) List of objects
                        d) GvShapes object
                        e) GvShapesLayer object (requires view)
                        f) None (to clear the grid or just show titles)

                view- only supplied for GvShapesLayer

                subset- subset to initialize the display with (a list of
                        source row indices). [optional]

                members, titles, editables, formats, types,
                nodata, justify, title_justify, force_width,
                force_width_chars, entry_chars- initial defining
                        column information (see define_columns). [optional].

                expose- whether or not to expose the grid immediately (1
                        to expose, 0 not to).  Defaults to 0.

                redefine_columns- set to 0 if column definitions should not
                                  change (if it is 0, then members, titles,
                                  etc. will be ignored).  If this is set to
                                  0, it is the calling function's
                                  responsibility to ensure that the columns
                                  are still valid for the new source.
                                  Defaults to 1.


        """
        if self.src is not None:
            self.clear()

        self.subset=subset
        self.bCalcAdjustments = True

        if ((source is None) or
            ((type(source) == type((1,))) and (len(source) == 0))):
            print "here1"
            # put tuples in with None rather than LISTUNDEF because
            # a tuple cannot be updated, so an empty tuple is
            # equivalent to None for display.
            self.src=None
            self.src_type=SRC_NONE
            if redefine_columns == 1:
                self._ColumnDefs=[]
                if members is not None:
                    self.define_columns(members,titles,editables,formats,types,
                            nodata,justify,title_justify,force_width,
                            force_width_chars, entry_chars,
                            expose=0)
            else:
                # Reset g_columns to previous number
                # of columns so titles are displayed
                self.g_columns=len(self._ColumnDefs)
                self._update_column_widths(expose=0)

            if expose == 1:
                self.expose()
            return

        if (type(source) == type([1,])) and (len(source) == 0):
            self.src=source
            self.src_type = SRC_LISTUNDEF
            if redefine_columns == 1:
                self._ColumnDefs=[]
                if (members is not None):
                    self.define_columns(members,titles,editables,formats,types,
                            nodata,justify,title_justify,force_width,
                            force_width_chars, entry_chars,
                            expose=expose)
            else:
                # Reset g_cols to previous number
                # of columns so titles are displayed
                self.g_columns=len(self._ColumnDefs)
                self._update_column_widths(expose=0)

            if expose == 1:
                self.expose()
            return

        if type(source) == type((1,)):
            source=list(source)

        if type(source) == type(Numeric.ones((4,4))):
            shp=Numeric.shape(source)
            if len(shp) == 1:
                self.src=Numeric.reshape(source,(1,shp[0]))
            elif len(shp) == 2:
                self.src=source
            else:
                txt='pgugrid: only 1 or 2-D Numeric arrays are supported'
                raise AttributeError,txt
            self.src_type=SRC_NUMERIC
        elif ((type(source) == type([])) and
              (type(source[0]) == type([]))):
            print "here3"
            self.src=source
            self.src_type=SRC_LISTLIST
        elif ((type(source) == type([])) and
              (type(source[0]) == type((1,)))):
            osrc=source
            source=[]
            for nexttuple in osrc:
                source.append(list(nexttuple))
            self.src=source
            self.src_type=SRC_LISTLIST
        elif type(source) == type([]):
            self.src=source
            self.src_type=SRC_LISTOBJ
        elif hasattr(source,'__len__'):
            self.src=source
            self.src_type=SRC_SHAPES
            self.source_changed_id=self.src.connect('changed',
                                                    self.refresh)
        else:
            try:
                self.src=source.get_parent()
                self.src_type=SRC_SHAPESLAYER

                self.source_changed_id=self.src.connect('changed',
                                                    self.refresh)
                self.layer=source
                self.layer_selection_changed_id = \
                     self.layer.connect('selection-changed',
                                        self.layer_selection_cb)
                self.layer_subselection_changed_id = \
                     self.layer.connect('subselection-changed',
                                        self.layer_subselection_cb)
                self.layer_teardown_id = \
                self.layer.connect('teardown',self.clear_and_expose)

                if view is None:
                    txt='pgugrid: if source is a shapeslayer, a viewarea\n'
                    txt=txt+'must be supplied.'
                    raise AttributeError,txt

                self.view=view
            except:
                txt='pgugrid: source must be one of:\n'
                txt=txt+'a) 1 or 2-D Numeric python array\n'
                txt=txt+'b) List of same-length lists\n'
                txt=txt+'c) List of objects with common elements\n'
                txt=txt+'d) GvShapes object\n'
                txt=txt+'e) GvShapesLayer object\n'
                raise AttributeError,txt


        if ((view is not None) and (self.src_type != SRC_SHAPESLAYER)):
            txt='pgugrid: view updates are only supported for\n'
            txt=txt+'the shapeslayer source datatype.'
            raise AttributeError,txt


        self._generate_row_indices()

        if redefine_columns == 1:
            self.define_columns(members,titles,editables,formats,types,
                                nodata,justify,title_justify,force_width,
                                force_width_chars, entry_chars,
                                expose=expose)
        else:
            self._update_column_widths(expose=expose)


        if self.src_type == SRC_SHAPESLAYER:
            # Make sure grid is initialized
            # to proper selection settings
            self.layer_selection_cb(self.layer)
            self.layer_subselection_cb(self.layer)

    def set_subset(self,indices=None,expose=1):
        """ Only display a subset of the items in the
            grid.

            Parameters:
                indices- a list of indices to source
                         for display.  Set to None
                         to display all rows.

                expose- 0 (do not redisplay grid) or
                        1 (redisplay grid).  Defaults
                        to 1.
        """

        if indices is not None:
            # Store a copy of the list so it cannot
            # be updated from outside
            self.subset=list(indices)
        else:
            self.subset=None

        self._generate_row_indices()

        # clear any selections not in the current subset
        if self.subset is not None:
            usrows=[]
            uscells=[]
            for item in self.selected_rows:
                if item not in self.subset:
                    usrows.append(item)
            for cell in self.selected_cells:
                if cell[0] not in self.subset:
                    uscells.append(cell)

            if len(usrows) > 0:
                self.unselect_rows(usrows,expose=0)
                Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))
                if self.src_type == SRC_SHAPESLAYER:
                    self.layer.display_change()

            if len(uscells) > 0:
                self.unselect_cells(uscells,expose=0)
                Signaler.notify(self, 'cell-selection-changed',
                            tuple(self.selected_cells))

        if expose == 1:
            self.expose()

    def get_selected_row_indices(self,*args):
        return tuple(self.selected_rows)

    def get_unselected_row_indices(self,*args):
        unsel=1-self.row_selectstate
        unselected=Numeric.compress(unsel > 0, Numeric.arange(len(unsel)))
        return tuple(unselected)

    def get_selected_column_indices(self,*args):
        return tuple(self.selected_columns)

    def get_unselected_column_indices(self,*args):
        unsel=1-self.column_selectstate
        unselected=Numeric.compress(unsel > 0, Numeric.arange(len(unsel)))
        return tuple(unselected)

    def get_selected_cell_indices(self,*args):
        return tuple(self.selected_cells)

    def get_source(self,*args):
        """ Use to get the underlying source.  This returns the
            original source, not a copy, so should normally only be used
            in read-only fashion.
        """
        return self.src

    def start_cell_edit(self,cell):
        """ Start editing new cell. """
        if self.editing_cell is not None:
            self.end_cell_edit()

        if self._ColumnDefs[cell[1]].editable == 0:
            # Column is not editable.
            self.editing_cell=None
            self._layout.move(self.editbox,5000,5000)
            return

        if self.opts.config[4] not in [4,5,6,7]:
            # Rows are not changeable.
            self.editing_cell=None
            self._layout.move(self.editbox,5000,5000)
            return

        self.editing_cell=cell
        self.reset_startrow(cell[0])
        self.reset_startcolumn(cell[1])

        # Make sure only currently edited cell is selected
        if ((len(self.selected_cells) == 0) or
            (len(self.selected_cells) > 1) or
            (self.selected_cells[0][0] != cell[0]) or
            (self.selected_cells[0][1] != cell[1])):
            self.select_cell(cell,clearfirst=1,expose=1)

        cwidth=self._ColumnDefs[cell[1]].width
        self.editbox.set_size_request(cwidth,self.row_height)
        self.editbox.set_max_length(self._ColumnDefs[cell[1]].entry_chars)
        self.editbox.set_text(self._get_datastr(cell[0],cell[1]))
        self.editbox.set_position(len(self._get_datastr(cell[0],cell[1])))
        locx=self._ColumnDefs[cell[1]].start_x-\
                      self._ColumnDefs[self.start_column].start_x+\
                      self.row_title_width+1
        locy=self.column_title_height+2+\
              ((self.src2row[cell[0]]-self.start_row)*(self.row_height+1))
        if self.opts.show_row_titles == 1:
            locy=locy+1

        self._layout.move(self.editbox,locx,locy)

    def cancel_cell_edit(self):
        """ Cancel the current cell edit, not saving changes. """
        self.editing_cell=None
        self._layout.move(self.editbox,5000,5000)

    def end_cell_edit(self):
        """ End current cell edit, saving changes. """
        if self.editing_cell is not None:
            newval=self.editbox.get_text()
            # TO DO: type checking (probably in set datastr itself though)
            self._set_datastr(self.editing_cell[0],self.editing_cell[1],newval)
            Signaler.notify(self, 'cell-changed',tuple(self.editing_cell))

            self.editing_cell=None
            self._layout.move(self.editbox,5000,5000)

    def entry_key_press(self,entry,event):
        """ Edit box had a key press event.  If enter was pressed,
            save changes to source and go on to next cell down (if
            there is one).  If tab was pressed, save and go on to next
            cell across.  If escape was pressed, cancel cell editing without
            saving changes.
        """

        # If a key was pressed, clear the information used
        # to detect "double" click (hack- see info in _initialize_settings
        # function)
        self.last_click_cell=None
        self.last_click_row=None
        self.last_click_column=None
        self.last_click_button=None
        # End of hack

        if self.editing_cell is None:
            return 

        if event.keyval == gtk.keysyms.Return:
            last_cell=self.editing_cell
            self.end_cell_edit()
            next_cell=(self.src2row[last_cell[0]]+1,last_cell[1])
            if next_cell[0] < len(self.row2src):
                next_cell=(self.row2src[next_cell[0]],last_cell[1])
                self.start_cell_edit(next_cell)
            else:
                self.unselect_all_cells()
                self._area.grab_focus()
        elif event.keyval == gtk.keysyms.Tab: 
            last_cell=self.editing_cell
            self.end_cell_edit()
            next_cell=(last_cell[0],last_cell[1]+1)
            if next_cell[1] >= len(self._ColumnDefs):
                n_row=self.src2row[last_cell[0]]+1
                if n_row >= len(self.row2src):
                    next_row=self.row2src[0]
                else:
                    next_row=self.row2src[n_row]
                next_cell=(next_row,0)

            if self._ColumnDefs[next_cell[1]].editable == 1:  
                self.start_cell_edit(next_cell)
            else:
                self.unselect_all_cells()

            # This next line avoids the next tab
            # being treated as part of the text
            # on alternate tabs (ie. jumping
            # from start to end within text
            # rather than resetting the
            # editable cell
            self._area.grab_focus()

        elif event.keyval == gtk.keysyms.Escape:
            self.cancel_cell_edit()
            self.unselect_all_cells()
            self._area.grab_focus()

    def area_key_press(self,area,event):
        """ If escape is pressed, clear all current cell selections
            and cancel any editing."""
        # If a key was pressed, clear the information used
        # to detect "double" click (hack- see info in _initialize_settings
        # function)
        self.last_click_cell=None
        self.last_click_row=None
        self.last_click_column=None
        self.last_click_button=None
        # End of hack

        if event.keyval == gtk.keysyms.Escape:
            self.cancel_cell_edit()
            self.unselect_all_cells()
        elif ((event.keyval == gtk.keysyms.BackSpace) or
              (event.keyval == gtk.keysyms.Delete)):
            if self.opts.config[4] in [1,3,5,7]:
                # row deletion supported by configuration
                rows=self.get_selected_row_indices()
                if len(rows) > 0:
                    self.delete_rows(rows)

    def _generate_row_indices(self):

        """ Create row index mappings.
            Source and grid are both indexed
            from 0.  Initially, all non-None source rows
            are displayed.  If a subset is specified,
            row2src will be subset minus any None's
            contained within the subset list.  Otherwise,
            row2src will map all non-None rows of source
            onto the grid.
        """

        # TO DO: Incorporate sorting property, select logic
        # (want to retain selections where possible)
        self.src2row=[]
        self.row2src=[]

        if self.src_type in [SRC_NONE,SRC_LISTUNDEF]:
            return

        if self.src_type == SRC_NUMERIC:
            # Numeric arrays don't have rows of None
            if self.subset is None:
                self.src2row=range(Numeric.shape(self.src)[0])
                self.row2src=range(Numeric.shape(self.src)[0])
            else:
                grididx=0
                for idx in range(Numeric.shape(self.src)[0]):
                    if idx in self.subset:
                        self.src2row.append(grididx)
                        grididx=grididx+1
                        self.row2src.append(idx)
                    else:
                        self.src2row.append(-1)

        else:
            if self.subset is None:
                grididx=0
                for idx in range(len(self.src)):
                    if self.src[idx] != None:
                        self.src2row.append(grididx)
                        grididx=grididx+1
                        self.row2src.append(idx)
                    else:
                        self.src2row.append(-2)
            else:
                grididx=0
                for idx in range(len(self.src)):
                    if idx in self.subset:
                        if self.src[idx] is None:
                            self.src2row.append(-2)
                        else:
                            self.src2row.append(grididx)
                            grididx=grididx+1
                            self.row2src.append(idx)
                    elif self.src[idx] != None:
                        # row is not None, but it
                        # isn't in subset
                        self.src2row.append(-1)
                    else:
                        self.src2row.append(-2)

        self.s_rows=len(self.src2row)
        self.g_rows=len(self.row2src)

        # If selection state has not been specified, initialize
        # to zeros.  If selection state has been specified and
        # source is the same length as before,
        # leave select state alone.  Otherwise,
        # reset row and cell matrices.
        if ((self.row_selectstate is None) or
            (len(self.row_selectstate) != self.s_rows)):
            self.row_selectstate=Numeric.zeros((self.s_rows,))
            self.cell_selectstate=Numeric.zeros((self.s_rows,self.g_columns))
            self.last_selected_row=None
            self.last_toggled_row=None
            self.last_selected_cell=None
            self.last_toggled_cell=None

        # If a sort column has been specified, do sorting
        if self.sort_column != -1:
            self.sort_by_column()

        if self.last_selected_row is not None:
            self.reset_startrow(self.last_selected_row)

        if self.opts.show_row_titles == 1:
            if (self.title_font is not None):
                self.row_title_width=self.title_font.string_width(
                    str(len(self.src2row)))+2*self.pad
            else:
                self.row_title_width=0
        else:
            self.row_title_width=0

        # Row colouring:
        # If no style index exists already, initialize
        # all rows to the default style.
        #
        # If the source is a shapes or shapes layer
        # object and a row style index exists that is
        # less than the length of the length of the
        # source, some rows have been added to the end
        # of the source (new shapes are always appended
        # to gvshapes; there are no functions for
        # inserting) so keep the existing row colours
        # the same and add more of the default colour
        # to the end (shapes are always appended).
        # In the shapes/shapeslayer cases, the rows of the 
        # source are just set to None when they are
        # deleted, and the list only shrinks if the
        # last shape is deleted (it shrinks until it
        # encounters a non-None shape).  This means that
        # if the row style index is longer than the source,
        # the row style index will still be valid for the
        # length of the source, and can just be truncated.
        #
        # For all the other types of objects (lists, tuples,
        # arrays), no assumptions can be made about how to
        # update the row style index if its length differs
        # from that of the source because rows can be
        # deleted/inserted at any point.  In these cases,
        # the row style index will be reset to the default
        # values for the length of the source if the two
        # differ in length; if they are the same, the
        # colours are left alone.  For these objects,
        # internal pgugrid functions such as delete_rows
        # should update the style list appropriately before
        # the row indices are regenerated so that
        # colours are maintained; if the source is updated
        # externally, the colours will be reset to the
        # default.
        if self.src_type in [SRC_SHAPES,SRC_SHAPESLAYER]: 
            if (self.row_style_index is None):
                self.row_style_index=Numeric.zeros((self.s_rows,))
            elif (len(self.row_style_index) > self.s_rows):
                self.row_style_index=self.row_style_index[:self.s_rows]
            elif (len(self.row_style_index) < self.s_rows):
                temp=self.row_style_index
                self.row_style_index=Numeric.zeros((self.s_rows,))
                self.row_style_index[:len(temp)]=temp
        else:
            if ((self.row_style_index is None) or
                (len(self.row_style_index) != self.s_rows)):
                self.row_style_index=Numeric.zeros((self.s_rows,))

        # Below: if row title styles are set, add code here:

        self.bCalcAdjustments=True

    def add_style(self,style_or_tuple):
        """ Add a style to the list of styles available to
            expose in the grid.  style_or_tuple may be either
            a GtkStyle, or a tuple consisting of
            (bg_gc_normal,fg_gc_normal,bg_gc_selected,fg_gc_selected,
             bg_gc_insensitive,fg_gc_insensitive),
            where bg=background, fg=foreground. Not all
            of these are used yet (may never be).
            Currently, the grid only distinguishes between
            selected and unselected rows and columns.
            Cell selection colours follow the ROW selection
            colours.  If a row and column are simultaneously
            selected, the column selection colours will override the
            row selection colours.
            Each of these entries should be either None
            (then the default value is used) or a tuple
            of 3 integer (0-65535) values.
            Returns the index of the new style.  
        """

        if type(style_or_tuple) == type(self.get_style()):
            self.style_list.append(style_or_tuple.copy())
            return len(self.style_list)-1
        else:
            # Style will be created at next expose (can't
            # be created if no expose has taken place)
            idx=len(self.style_list)
            if self.styles_to_add is None:
                self.styles_to_add=[]

            self.styles_to_add.append((idx,style_or_tuple))
            self.style_list.append(None)
            return idx

    def _add_style(self,cidx,ctuple):
        """ Internal function that actually creates the requested
            style.
        """
        if self.style_list[cidx] is None:
            style=self.get_style()
        else:
            # use existing style for defaults if it is
            # present
            style = self.style_list[cidx]

        nstyle=style.copy()
        for idx in range(max(len(ctuple),6)):
            item=ctuple[idx]
            if item is None:
                if (idx == 0) and (style.bg_gc[gtk.STATE_NORMAL] is not None):
                    nstyle.bg_gc[gtk.STATE_NORMAL] = \
                                 style.bg_gc[gtk.STATE_NORMAL]
                elif (idx == 1) and (style.fg_gc[gtk.STATE_NORMAL] is not None):
                    nstyle.fg_gc[gtk.STATE_NORMAL] = \
                                 style.fg_gc[gtk.STATE_NORMAL]
                elif (idx == 2) and (style.bg_gc[gtk.STATE_SELECTED] is not None):
                    nstyle.bg_gc[gtk.STATE_SELECTED] = \
                                 style.bg_gc[gtk.STATE_SELECTED]
                elif (idx == 3) and (style.fg_gc[gtk.STATE_SELECTED] is not None):
                    nstyle.fg_gc[gtk.STATE_SELECTED] = \
                                 style.fg_gc[gtk.STATE_SELECTED]
                elif (idx == 4) and (style.bg_gc[gtk.STATE_INSENSITIVE] is not None):
                    nstyle.bg_gc[gtk.STATE_INSENSITIVE] = \
                                 style.bg_gc[gtk.STATE_INSENSITIVE]
                elif (idx == 5) and (style.fg_gc[gtk.STATE_INSENSITIVE] is not None):
                    nstyle.fg_gc[gtk.STATE_INSENSITIVE] = \
                                 style.fg_gc[gtk.STATE_INSENSITIVE]

                continue

            if idx == 0:
                ngc=self._get_new_gc(item,style.bg_gc[gtk.STATE_NORMAL])
                nstyle.bg_gc[gtk.STATE_NORMAL]=ngc
            elif idx == 1:
                ngc=self._get_new_gc(item,style.fg_gc[gtk.STATE_NORMAL])
                nstyle.fg_gc[gtk.STATE_NORMAL]=ngc
            elif idx == 2:
                ngc=self._get_new_gc(item,style.bg_gc[gtk.STATE_SELECTED])
                nstyle.bg_gc[gtk.STATE_SELECTED]=ngc
            elif idx == 3:
                ngc=self._get_new_gc(item,style.fg_gc[gtk.STATE_SELECTED])
                nstyle.fg_gc[gtk.STATE_SELECTED]=ngc
            elif idx == 4:
                ngc=self._get_new_gc(item,
                                     style.bg_gc[gtk.STATE_INSENSITIVE])
                nstyle.bg_gc[gtk.STATE_INSENSITIVE]=ngc
            elif idx == 5:
                ngc=self._get_new_gc(item,
                                     style.fg_gc[gtk.STATE_INSENSITIVE])
                nstyle.fg_gc[gtk.STATE_INSENSITIVE]=ngc

        # None placeholder was created when user called add_style to
        # request the new style.  Here, just replace the None.
        self.style_list[cidx]=nstyle

    def _get_new_gc(self,color_tuple, ref_gc):
        """ Get a new graphics context for a style. """
        # WARNING: Don't try printing out the gc's foreground and
        # background values- it seems that the printed values are
        # unrelated to the actual set values.  Setting them does
        # seem to work though.
        cmap=self.get_colormap()
        new_color=cmap.alloc_color(color_tuple[0],color_tuple[1],color_tuple[2])
        if ref_gc is None:
            ref_gc=self.get_style().white_gc

        ngc=self.window.new_gc(foreground=new_color,
        #                             background=new_color)
            background=new_color,font=self.get_style().get_font(), fill=ref_gc.fill,
            subwindow_mode=ref_gc.subwindow_mode,
	    ts_x_origin=ref_gc.ts_x_origin, ts_y_origin=ref_gc.ts_y_origin,
      	    clip_x_origin=ref_gc.clip_x_origin,
            clip_y_origin=ref_gc.clip_y_origin,
	    line_width=ref_gc.line_width, line_style=ref_gc.line_style,
            cap_style=ref_gc.cap_style,
	    join_style=ref_gc.join_style)

        return ngc

    def set_line_drawing(self,rlines=2, clines=2, expose=1):
        """ Set which lines in the grid are drawn:
            Inputs:
                rlines- 0 for no lines between rows
                        1 for lines between row titles
                        2 for lines under whole rows

                clines- 0 for no lines between columns
                        1 for lines between column titles
                        2 for lines under whole columns


        """
        self.draw_row_lines = rlines
        self.draw_col_lines = clines
        if expose == 1:
            self.expose()

    def set_default_style(self,tuple):
        """ Set the default style:  a tuple consisting of
            (bg_gc_normal,fg_gc_normal,bg_gc_selected,fg_gc_selected,
             bg_gc_insensitive,fg_gc_insensitive),
            where bg=background, fg=foreground. Not all
            of these are used yet (may never be).
            Each of these entries should be either None
            (then the default value is used) or a tuple
            of 3 integer (0-65535) values.  
        """
        self.default_style_reset_flag = 1
        self.default_style = tuple

    def set_default_row_title_style(self,tuple):
        """ Set the default row title style:  a tuple consisting of
            (bg_gc_normal,fg_gc_normal,bg_gc_selected,fg_gc_selected,
             bg_gc_insensitive,fg_gc_insensitive),
            where bg=background, fg=foreground. Not all
            of these are used yet (may never be).
            Each of these entries should be either None
            (then the default value is used) or a tuple
            of 3 integer (0-65535) values.  
        """
        self.default_row_title_style_reset_flag = 1
        self.default_row_title_style = tuple

    def set_default_col_title_style(self,tuple):
        """ Set the default column title style:  a tuple consisting of
            (bg_gc_normal,fg_gc_normal,bg_gc_selected,fg_gc_selected,
             bg_gc_insensitive,fg_gc_insensitive),
            where bg=background, fg=foreground. Not all
            of these are used yet (may never be).
            Each of these entries should be either None
            (then the default value is used) or a tuple
            of 3 integer (0-65535) values.  
        """
        self.default_col_title_style_reset_flag = 1
        self.default_col_title_style = tuple

    def set_default_selection_colour(self,ctuple):
        """ Set the default selection colour: a tuple
            of three integers.
        """
        self.set_default_style((None,None,ctuple,None,None,None))

    def set_row_style(self,row_index,style_index):
        """ Assign a style index to a row. row_index
            is either a single integer index or a list
            of indices.  If it is -1, all rows will
            be set to have style style_index.  If it is
            a list, the rows (in source coordinates) in
            that list will have that style.  If it is
            a single integer that is > -1, only that
            row will be set to have that style.
            style_index is always an integer.
        """

        if type(row_index) == type([]):
            for row in row_index:
                self.row_style_index[row]=style_index
        elif row_index == -1:
            self.row_style_index=\
                style_index*Numeric.ones((self.s_rows,))
        else:
            self.row_style_index[row_index]=style_index


    def set_column_style(self,column_index,style_index):
        """ Assign a style index to a column. column_index
            is either a single integer index or a list
            of indices.  If it is -1, all columns will
            be set to have style style_index.  If it is
            a list, the columns (in grid coordinates) in
            that list will have that style.  If it is
            a single integer that is > -1, only that
            column will be set to have that style.
            style_index is always an integer.  Note that
            only the selected states of column styles
            will be seen- in drawing, the unselected
            row color overrides the unselected column
            color.
        """

        if type(column_index) == type([]):
            for column in column_index:
                self.column_style_index[column]=style_index
        elif column_index == -1:
            self.column_style_index=\
                style_index*Numeric.ones((self.g_columns,))
        else:
            self.column_style_index[column_index]=style_index



    def set_row_title_type(self,type,expose=1):
        """Define whether row titles should reflect grid
           row number, or underlying source index.
           Parameters:
               type- either 'grid' (to show grid row) or
                     'source' (to show source row index).
               expose- 1 if pgugrid should be redrawn
                       immediately; 0 if not.  Defaults
                       to 1.
        """
        self.opts.set_row_title_type(type)

        if expose == 1:
            self.expose()

    def define_columns(self,members=None,titles=None,editables=None,
                       formats=None,types=None,nodata=None,
                       justify=None,title_justify=None,
                       force_width=None,force_width_chars=None,
                       entry_chars=None,expose=1):
        """ Define which columns to include in display.
            Parameters:
                members- Relates the column to the underlying source.
                         Is either None (use defaults) or a list.
                         List contains elements described here:
                         SRC_NUMERIC: column index (integer)
                         SRC_SHAPES: shape property
                         SRC_SHAPESLAYER: shape property
                         SRC_LISTLIST: sublist index
                         SRC_LISTOBJ: object member, or None if the
                                      object is a string, float, integer,
                                      or complex.

                title- Describes the display title for each column.
                       Is either None or a list of strings.  If list, the
                       list must have a one-to-one correspondence
                       to the members list (if members list is None,
                       the length of this list must be the same as
                       the number of members detected by default).

                editables- Is either None (use global editable parameter
                           used in initial grid definition), 0 (no
                           editable columns), 1 (all editable columns),
                           or a list of 1's and zeros that has a one-to-one
                           correspondence with the members list.

                formats- Is either None (use default formatting), a single
                         expression that should be used to format all
                         columns, or a list of expressions that has a
                         one-to-one correspondence with the members
                         list.

                types- Is either None (try to auto-detect type), a
                       single type describing members of all columns,
                       or a list of types that has a one-to-one
                       correspondence with the members list.

                nodata- Is either None, a single string describing
                        the nodata string for members of all columns,
                        or a list of nodata strings that has a one-to-one
                        correspondence with the members list.

                justify- Is either None, a single integer to describe
                         all column justifications (0=right, 1=left,
                         2=center), or a list of integers (0-2)
                         that has a one-to-one correspondence with the
                         members list. Defaults to 0.

                title_justify- Is either None, a single integer to describe
                               all column title justifications (0=right,
                               1=left, 2=center), or a list of integers (0-2)
                               that has a one-to-one correspondence with the
                               members list. Defaults to 2.

                force_width- Used to force a particular column's width.
                             Is either None, a single width to describe
                             all column widths, or a list of widths
                             that has a one-to-one correspondence with the
                             members list. Defaults to None (auto-select
                             column widths). 

                force_width_chars- whether width is specified in terms of
                                   pixels (-1) or characters (> 0).  Is either
                                   None, a single value to describe all
                                   columns, or a list of values with a
                                   one-to-one correspondence with the
                                   members list.  Defaults to -1.

                entry_chars- number of characters to allow user to type
                             (only relevant if column is editable).  Is
                             either None, a single value to describe all
                             columns, or a list of values with a one-to-one
                             correspondence with the members list.  Defaults
                             to 90.
        """


        self._ColumnDefs=[]

        if (self.src_type in [SRC_NONE,SRC_LISTUNDEF]) and (members is None):
            return

        if type(members) == type((1,)):
            members=list(members)
        if type(titles) == type((1,)):
            titles=list(titles)
        if type(editables) == type((1,)):
            editables=list(editables)
        if type(formats) == type((1,)):
            formats=list(formats)
        if type(types) == type((1,)):
            types=list(types)
        if type(nodata) == type((1,)):
            nodata=list(nodata)
        if type(justify) == type((1,)):
            justify=list(justify)
        if type(title_justify) == type((1,)):
            title_justify=list(title_justify)
        if type(force_width) == type((1,)):
            force_width=list(force_width)
        if type(force_width_chars) == type((1,)):
            force_width_chars=list(force_width_chars)
        if type(entry_chars) == type((1,)):
            entry_chars=list(entry_chars)

        startx=self.row_title_width+1
        if self.opts.show_row_titles == 1:
            startx=startx+1

        if members is None:
            if self.src_type == SRC_NUMERIC:
                # Default: one column per array column, with
                # column number as the title              
                ci_mems=range(Numeric.shape(self.src)[1])
            elif ((self.src_type == SRC_SHAPES) or
                  (self.src_type == SRC_SHAPESLAYER)):
                ci_mems=[]
                schema=self.src.get_schema()
                if len(schema) > 0:
                    for item in schema:
                        ci_mems.append(item[0])
                else:
                    if len(self.src) > 0:
                        props=self.src[0].get_properties()
                        for ckey in props.keys():
                            ci_mems.append(ckey)
            elif (self.src_type == SRC_LISTLIST):
                ci_mems=range(len(self.src[0]))
            elif (self.src_type == SRC_LISTOBJ):
                if type(self.src[0]) == type(''):
                    ci_mems=[None]
                elif type(self.src[0]) == type(1):
                    ci_mems=[None]
                elif type(self.src[0]) == type(5.2):
                    ci_mems=[None]
                elif type(self.src[0]) == type(complex(1,1)):
                    ci_mems=[None]
                else:    
                    members=dir(self.src[0])
                    ci_mems=[]
                    for ckey in members:
                        cmem=eval('self.src[0].'+ckey)
                        oktypes=[type(''),type(1),type(5.2),type(complex(1,1))]
                        if type(cmem) in oktypes:
                            ci_mems.append(ckey)
        else:
            ci_mems=members

        ci_titles=[]
        if titles is None:
            if self.src_type in [SRC_NUMERIC,SRC_LISTLIST]:
                for idx in ci_mems:
                    ci_titles.append('Column '+str(idx))
            elif (self.src_type in
              [SRC_SHAPES,SRC_SHAPESLAYER,SRC_LISTOBJ,SRC_NONE,SRC_LISTUNDEF]):
                for mem in ci_mems:
                    if mem is not None:
                        ci_titles.append(mem)
                    else:
                        ci_titles.append('Column 0')

        elif type(titles) == type(''):
            for idx in ci_mems:
                ci_titles.append(titles)
        elif type(titles) == type([]):
            ci_titles=titles
            if len(ci_titles) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                                   'title per member!'
        else:
            raise RuntimeError,'define_columns: titles must be either '+\
                               'None, a string, or a list.'

        ci_edit=[]
        if editables is None:
            if self.opts.config[4] in [4,5,6,7]:
                editopts=1
            else:
                editopts=0

            for idx in ci_mems:
                ci_edit.append(editopts)

        elif editables in [0,1]:
            for idx in ci_mems:
                ci_edit.append(editables)
        elif type(editables) == type([]):
            ci_edit=editables
            if len(ci_edit) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                           'edit property per member if editables is a list!'
        else:
            raise RuntimeError,'define_columns: editables must be either '+\
                               'None, 0, 1, or a list.'


        ci_fmts=[]
        if formats is None:
            if ((self.src_type == SRC_SHAPES) or
                  (self.src_type == SRC_SHAPESLAYER)):
                schema=self.src.get_schema()
                schema_fmts={}
                for item in schema:
                    if item[1] == 'integer':
                        schema_fmts[item[0]]="%"+str(item[2])+"d"
                    elif item[1] == 'float':
                        schema_fmts[item[0]]="%"+str(item[2])+"."+\
                                              str(item[3])+"f"

                for idx in ci_mems:
                    if schema_fmts.has_key(idx):
                        ci_fmts.append(schema_fmts[idx])
                    else:
                        ci_fmts.append(None)

            else:
                for idx in ci_mems:
                    ci_fmts.append(None)
        elif type(formats) == type(''):
            for idx in ci_mems:
                ci_fmts.append(formats)
        elif type(formats) == type([]):
            ci_fmts=formats
            if len(ci_fmts) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                                   'format per member if formats is a list!'
        else:
            raise RuntimeError,'define_columns: formats must be either '+\
                               'None, a string, or a list.'

        ci_types=[]
        if types is None:
            if self.src_type == SRC_NUMERIC:
                #inttypes=['1','l','s','i','u','b','w']
                inttypes=[Numeric.Int,Numeric.Int0,Numeric.Int8,Numeric.Int16,
                          Numeric.Int32,Numeric.UnsignedInteger,Numeric.UInt8,
                          Numeric.UInt16,Numeric.UInt32]
                #complextypes=['D','F']
                complextypes=[Numeric.Complex,Numeric.Complex0,
                              Numeric.Complex8,
                              Numeric.Complex16,Numeric.Complex32,
                              Numeric.Complex64]
                if self.src.typecode() in inttypes:
                    ctype='integer'
                elif self.src.typecode() in complextypes:
                    ctype='complex'
                else:
                    ctype='float'

                for mem in ci_mems:
                    ci_types.append(ctype)

            elif self.src_type in [SRC_SHAPES,SRC_SHAPESLAYER]:
                schema=self.src.get_schema()
                sd={}
                for item in schema:
                    sd[item[0]]=item[1]
                sdkeys=sd.keys()    
                for mem in ci_mems:
                    if mem in sdkeys:
                        ci_types.append(sd[mem])
                    else:
                        ci_types.append('string')

            elif self.src_type == SRC_LISTLIST:
                for idx in ci_mems:
                    if type(self.src[0][idx]) == type(1.2):
                        ctype='float'
                    elif type(self.src[0][idx]) == type(1):
                        ctype='integer'
                    elif type(self.src[0][idx]) == type(complex(1,1)):
                        ctype='complex'
                    else:
                        ctype='string'
                    ci_types.append(ctype)

            elif self.src_type == SRC_LISTOBJ:
                if ci_mems[0] is None:
                    if type(self.src[0]) == type(''):
                        ci_types.append('string')
                    elif type(self.src[0]) == type(1):
                        ci_types.append('integer')
                    elif type(self.src[0]) == type(5.2):
                        ci_types.append('float')
                    elif type(self.src[0]) == type(complex(1,1)):
                        ci_types.append('complex')
                else:
                    for mem in ci_mems:
                        cmem=eval('self.src[0].'+mem)
                        if type(cmem) == type(complex(1,1)):
                            ci_types.append('complex')
                        elif type(cmem) == type(5.3):
                            ci_types.append('float')
                        elif type(cmem) == type(1):
                            ci_types.append('integer')
                        else:
                            ci_types.append('string')
            elif self.src_type in [SRC_NONE,SRC_LISTUNDEF]:
                for mem in ci_mems:
                    ci_types.append(None)

        elif type(types) == type(''):
            for idx in ci_mems:
                ci_types.append(types)
        elif type(types) == type([]):
            ci_types=list(types)
            if len(ci_types) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                                   'type per member if types is a list!'
        else:
            raise RuntimeError,'define_columns: types must be either '+\
                               'None, a string, or a list.'


        ci_nodata=[]
        if nodata is None:
            for idx in ci_mems:
                ci_nodata.append('')
        elif type(nodata) == type(''):
            for idx in ci_mems:
                ci_nodata.append(nodata)
        elif type(nodata) == type([]):
            ci_nodata=nodata
            if len(ci_nodata) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                                   'string per member if nodata is a list!'
        else:
            raise RuntimeError,'define_columns: nodata must be either '+\
                               'None, a string, or a list.'


        ci_just=[]
        if justify is None:
            for idx in ci_mems:
                ci_just.append(0)
        elif type(justify) == type(1):
            for idx in ci_mems:
                ci_just.append(justify)
        elif type(justify) == type([]):
            ci_just=justify
            if len(ci_just) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                                   'integer per member if justify is a list!'
        else:
            raise RuntimeError,'define_columns: justify must be either '+\
                               'None, a string, or a list.'


        ci_tjust=[]
        if title_justify is None:
            for idx in ci_mems:
                ci_tjust.append(2)
        elif type(title_justify) == type(1):
            for idx in ci_mems:
                ci_tjust.append(title_justify)
        elif type(title_justify) == type([]):
            ci_tjust=title_justify
            if len(ci_tjust) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                         'integer per member if title_justify is a list!'
        else:
            raise RuntimeError,'define_columns: title_justify must be '+\
                               'either None, a string, or a list.'

        ci_fwidth=[]
        if force_width is None:
            for idx in ci_mems:
                ci_fwidth.append(None)
        elif type(force_width) == type(1):
            for idx in ci_mems:
                ci_fwidth.append(force_width)
        elif type(force_width) == type([]):
            ci_fwidth=force_width
            if len(ci_fwidth) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                         'integer per member if force_width is a list!'
        else:
            raise RuntimeError,'define_columns: force_width must be '+\
                               'either None, a string, or a list.'

        ci_fwidthc=[]
        if force_width_chars is None:
            for idx in ci_mems:
                ci_fwidthc.append(-1)
        elif type(force_width_chars) == type(1):
            for idx in ci_mems:
                ci_fwidthc.append(force_width_chars)
        elif type(force_width_chars) == type([]):
            ci_fwidthc=force_width_chars
            if len(ci_fwidthc) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                         'integer per member if force_width_chars is a list!'
        else:
            raise RuntimeError,'define_columns: force_width_chars must be '+\
                               'either None, a string, or a list.'

        ci_entryc=[]
        if entry_chars is None:
            for idx in ci_mems:
                ci_entryc.append(90)
        elif type(entry_chars) == type(1):
            for idx in ci_mems:
                ci_entryc.append(entry_chars)
        elif type(entry_chars) == type([]):
            ci_entryc=entry_chars
            if len(ci_entryc) != len(ci_mems):
                raise RuntimeError,'define_columns: must specify one '+\
                         'integer per member if entry_chars is a list!'
        else:
            raise RuntimeError,'define_columns: entry_chars must be '+\
                               'either None, a string, or a list.'

        style = self.get_style()

        if self.title_font is None:
            try:
                self.title_font = gtk.load_font( self.title_font_spec )
            except:
                ##traceback.print_exc()
                self.title_font = style.get_font()

            if self.opts.show_column_titles == 1:    
                self.column_title_height = self.title_font.ascent + 2*self.pad
            else:
                self.column_title_height=0

            if self.opts.show_row_titles == 1:
                if (self.src is not None):
                    self.row_title_width=self.title_font.string_width(
                        str(len(self.src2row)))+2*self.pad
                else:
                    self.row_title_width=0  
            else:
                self.row_title_width=0                

        if self.cell_font is None:
            try:
                self.cell_font = gtk.load_font( self.cell_font_spec )
            except:
                ##traceback.print_exc()
                self.cell_font = style.get_font()
            self.row_height = self.cell_font.ascent + 2*self.pad

        for idx in range(len(ci_mems)):
            cwidth=30
            cnew=_column_info(ci_mems[idx],ci_titles[idx],ci_types[idx],
                              ci_fmts[idx],ci_edit[idx],ci_nodata[idx],
                              cwidth,startx,ci_just[idx],ci_tjust[idx],
                              ci_fwidth[idx], ci_fwidthc[idx], ci_entryc[idx])
            self._ColumnDefs.append(cnew)            
            # Cycle though first few rows to update column width
            if ((self._ColumnDefs[idx].force_width is not None) or
                (self._ColumnDefs[idx].force_width_chars > -1)):
                if self._ColumnDefs[idx].force_width_chars == -1:
                    cwidth = self._ColumnDefs[idx].force_width
                else:
                    # This assumes that 'W' has pretty much the maximum
                    # width for most fonts (rather than cycling through
                    # all possible characters).  May need to update...
                    cwidth = (max(self.title_font.string_width('W'),
                                 self.cell_font.string_width('W'))*
                         self._ColumnDefs[idx].force_width_chars) + 2*self.pad
                    self._ColumnDefs[idx].force_width = cwidth

            else:
                cwidth=self.title_font.string_width(self._ColumnDefs[idx].title)+\
                    2*self.pad
                tmprows=min([10,len(self.row2src)])
                for cr in range(tmprows):
                    cwidth=max([cwidth,self.cell_font.string_width(
                        self._get_datastr(cr,idx))+2*self.pad])
                cwidth=max(cwidth,30)


            self._ColumnDefs[idx].width=cwidth
            startx=startx+cwidth+1


        self.g_rows=len(self.row2src)    
        self.g_columns=len(self._ColumnDefs)

        self.column_widths=[]
        for item in self._ColumnDefs:
            self.column_widths.append(item.width)

        self.column_selectstate=Numeric.zeros((self.g_columns,))
        self.cell_selectstate=Numeric.zeros((self.s_rows,self.g_columns))
        self.last_selected_column=None
        self.last_selected_cell=None
        self.last_toggled_column=None
        self.last_toggled_cell=None

        self.column_style_index=Numeric.zeros((self.g_columns,))
        # Below: not used yet, but will be later
        #self.columntitle_style_index=Numeric.ones((self.g_columns,))

        self.bCalcAdjustments = True

        if expose == 1:
            self.expose()


    def _update_column_widths(self,expose=1):
        """ Update column widths for new source with same column defs. """

        style = self.get_style()

        if self.title_font is None:
            try:
                self.title_font = gtk.load_font( self.title_font_spec )
            except:
                ##traceback.print_exc()
                self.title_font = style.get_font()

            if self.opts.show_column_titles == 1:    
                self.column_title_height = self.title_font.ascent + 2*self.pad
            else:
                self.column_title_height=0

            if self.opts.show_row_titles == 1:
                if (self.src is not None):
                    self.row_title_width=self.title_font.string_width(
                        str(len(self.src2row)))+2*self.pad
                else:
                    self.row_title_width=0  
            else:
                self.row_title_width=0                

        if self.cell_font is None:
            try:
                self.cell_font = gtk.load_font( self.cell_font_spec )
            except:
                ##traceback.print_exc()
                self.cell_font = style.get_font()
            self.row_height = self.cell_font.ascent + 2*self.pad

        cwidth=30
        startx=self.row_title_width+1
        if self.opts.show_row_titles == 1:
            startx=startx+1
        for idx in range(len(self._ColumnDefs)):
            if (( self._ColumnDefs[idx].force_width is not None) or
                (self._ColumnDefs[idx].force_width_chars > -1)):
                if self._ColumnDefs[idx].force_width_chars == -1:
                    cwidth = self._ColumnDefs[idx].force_width
                else:
                    # This assumes that 'W' has pretty much the maximum
                    # width for most fonts (rather than cycling through
                    # all possible characters).  May need to update...
                    cwidth = (max(self.title_font.string_width('W'),
                                 self.cell_font.string_width('W'))*
                        self._ColumnDefs[idx].force_width_chars) + 2*self.pad
                    self._ColumnDefs[idx].force_width = cwidth
            else:
                cwidth=self.title_font.string_width(self._ColumnDefs[idx].title)+\
                    2*self.pad
                tmprows=min([10,len(self.row2src)])
                for cr in range(tmprows):
                    cwidth=max([cwidth,self.cell_font.string_width(
                        self._get_datastr(cr,idx))+2*self.pad])
                cwidth=max(cwidth,30)
            self._ColumnDefs[idx].width=cwidth
            self._ColumnDefs[idx].start_x=startx
            startx=startx+cwidth+1

        self.bCalcAdjustments = True  
        if expose == 1:
            self.expose()

    def _get_datastr(self,row,column):
        """ Get the underlying source data in cell row,column as a string. """
        cdata=self._get_data(row,column)

        if cdata is None:
            cdata=self._ColumnDefs[column].nodata
            return cdata

        if self._ColumnDefs[column].format in ['',None]:
            return str(cdata)
        else:
            return self._ColumnDefs[column].format % cdata

    def _get_data(self,row,column):
        """ Get the source data in cell row,column. """
        if self.src_type == SRC_NUMERIC:
            return self.src[row,self._ColumnDefs[column].member]
        elif self.src_type == SRC_SHAPES:
            if self.src[row] is not None:
                datastr = self.src[row].get_property(
                    self._ColumnDefs[column].member)
            else:
                return None

            if datastr is None:
                return None

            if self._ColumnDefs[column].type == 'float':
                data=float(datastr)
            elif self._ColumnDefs[column].type == 'integer':
                data=int(datastr)
            else:
                data=datastr

            return data

        elif self.src_type == SRC_SHAPESLAYER:
            if self.src[row] is not None:
                datastr = self.src[row].get_property(
                    self._ColumnDefs[column].member)
            else:
                return None

            if datastr is None:
                return None

            if self._ColumnDefs[column].type == 'float':
                data=float(datastr)
            elif self._ColumnDefs[column].type == 'integer':
                data=int(datastr)
            else:
                data=datastr

            return data

        elif self.src_type == SRC_LISTLIST:
            if self.src[row] is not None:
                datastr= self.src[row][self._ColumnDefs[column].member]
            else:
                return None

            if self._ColumnDefs[column].type == 'float':
                data=float(datastr)
            elif self._ColumnDefs[column].type == 'integer':
                data=int(datastr)
            elif self._ColumnDefs[column].type == 'complex':
                data=complex(datastr)
            else:
                data=datastr

            return data

        elif self.src_type == SRC_LISTOBJ:
            if self._ColumnDefs[column].member is None:
                datastr=self.src[row]
            else:
                datastr = eval('self.src[row].'+
                               self._ColumnDefs[column].member)

            if self._ColumnDefs[column].type == 'float':
                data=float(datastr)
            elif self._ColumnDefs[column].type == 'integer':
                data=int(datastr)
            elif self._ColumnDefs[column].type == 'complex':
                data=complex(datastr)
            else:
                data=datastr

            return data

    def get_cell_data(self,row,column):
        """ Get the source data in cell (source) row, (grid) column. """
        return self._get_data(row,column)

    def get_cell_data_string(self,row,column):
        """ Get the source data in cell (source) row,(grid) column
            as a string. """
        return self._get_datastr(row,column)

    def set_cell_data(self,row,column,value):
        """ Set the source data in cell (source) row, (grid) column. """
        return self._set_data(row,column,value)

    def set_cell_data_string(self,row,column,value):
        """ Set the source data string in cell (source) row,(grid) column. """
        return self._set_datastr(row,column,value)

    def _set_datastr(self,row,column,value):
        """ Set the source data in cell row,column to value.
            Note that value is entered as a string that
            must be converted to the required type."""

        if self._ColumnDefs[column].type == 'string':
            nvalue=value
        elif self._ColumnDefs[column].type == 'integer':
            try:
                nvalue=int(value)
            except:
                ##traceback.print_exc()
                # If empty string was entered, user may have
                # clicked on an empty cell and not typed
                # anything, so don't set the value or send
                # an error
                if len(value) == 0:
                    return

                gvutils.error('Invalid data entry.  Integer required.')
                return

        elif self._ColumnDefs[column].type == 'float':
            try:
                nvalue=float(value)
            except:
                ##traceback.print_exc()
                if len(value) == 0:
                    return

                gvutils.error('Invalid data entry.  Float required.')
                return

        elif self._ColumnDefs[column].type == 'complex':
            try:
                nvalue=complex(value)
            except:
                print traceback.print_exc()
                if len(value) == 0:
                    return

                gvutils.error('Invalid data entry.  Complex required.')
                return

        self._set_data(row,column,nvalue)

    def _set_data(self,row,column,value):
        """ Set the source data in cell row,column to value.
            Necessary conversions have already taken
            place (use _set_datastr if value is still a
            string that needs to be converted).
        """
        if self.src_type == SRC_NUMERIC:
            try:
                self.src[row,self._ColumnDefs[column].member]=value
            except:
                print traceback.print_exc()
                dtype=self.src.typecode()
                txt='Invalid entry for array of typecode '+dtype
                gvutils.error(txt)                
        elif self.src_type == SRC_SHAPES:
            s1=str(value).strip()
            pval=self.src[row].get_property(
                self._ColumnDefs[column].member)
            if pval is not None:
                s2=pval.strip()
            else:
                s2=None
                if s1 == '':
                    return

            if s1 != s2:
                # The copying is necessary for undo to work properly
                shape=self.src[row].copy()
                shape.set_property(self._ColumnDefs[column].member,
                                       s1)
                # avoid regenerating row indices
                self.src.handler_block(self.source_changed_id)
                self.src[row]=shape
                self.src.handler_unblock(self.source_changed_id)
        elif self.src_type == SRC_SHAPESLAYER:
            s1=str(value).strip()
            pval=self.src[row].get_property(
                self._ColumnDefs[column].member)
            if pval is not None:
                s2=pval.strip()
            else:
                s2=None
                if s1 == '':
                    return

            if s1 != s2:
                shape=self.src[row].copy()
                shape.set_property(self._ColumnDefs[column].member,
                                           s1)
                self.src.handler_block(self.source_changed_id)
                self.src[row]=shape
                self.src.handler_unblock(self.source_changed_id)
        elif self.src_type == SRC_LISTLIST:
            self.src[row][self._ColumnDefs[column].member]=value
        elif self.src_type == SRC_LISTOBJ:
            if self._ColumnDefs[column].member is None:
                self.src[row]=value
            else:
                setattr(self.src[row],self._ColumnDefs[column].member,value)


    def translate_view_to_row(self,row):
        """ If source is a shapeslayer, translate the view
            to center on the row'th source shape and select
            that row if it isn't already selected and selection
            is enabled.
        """

        if self.view is not None:
            if row > (len(self.src)-1):
                raise RuntimeError,'translate_row_to_view: tried to '+\
                      'translate view to center on non-existent node.'

            cnode=self.src[row].get_node()
            self.view.set_translation(-cnode[0],-cnode[1])
            if self.row_selectstate[row] == 0:
                if self.opts.selection_info.row == 1:
                    self.select_row(row,clearfirst=1,expose=1)
                elif self.opts.selection_info.row == 2:
                    self.select_row(row,clearfirst=0,expose=1)

    def delete_row(self,row):
        """ Delete source row row.  Note: this function will
            reset the source in the Numerical array case 
            (necessary because array must be re-allocated when
            it changes size).
        """

        self.delete_rows([row])

    def delete_rows(self,row_list):
        """ Delete source rows in row_list.  Note: this function will
            reset the source in the Numerical array case 
            (necessary because array must be re-allocated when
            it changes size).
        """

        if type(row_list) == type((1,)):
            row_list=list(row_list)

        if len(row_list) == 0:
            return

        self.bCalcAdjustments=True

        if self.src_type in [SRC_SHAPES,SRC_SHAPESLAYER]:
            self.src.delete_shapes(row_list)

        elif self.src_type in [SRC_LISTLIST,SRC_LISTOBJ]:
            # sort the row list in descending order so that
            # deleting one row won't alter the indices
            # of the next one.
            row_list.sort()
            row_list.reverse()
            rstyle_index=list(self.row_style_index)
            for item in row_list:
                self.src.pop(item)
                rstyle_index.pop(item)

            self.row_style_index=Numeric.array(rstyle_index)

            self.refresh()
        else:
            row_list.sort()
            newrows=self.src.shape[0]-len(row_list)
            cols=self.src.shape[1]
            rm_rows=0
            first_row=0
            newarr=Numeric.zeros((newrows,cols),self.src.typecode())
            for item in row_list:
                newarr[first_row:item-rm_rows,:]=\
                             self.src[first_row+rm_rows:item,:]
                first_row=item-rm_rows
                rm_rows=rm_rows+1

            newarr[first_row:newrows,:]=self.src[first_row+rm_rows:]

            # Update row style list (colours)
            row_list.reverse()
            rstyle_index=list(self.row_style_index)

            for item in row_list:
                rstyle_index.pop(item)

            self.row_style_index=Numeric.array(rstyle_index)

            self.src=newarr
            self.refresh()

        Signaler.notify(self, 'rows-deleted',row_list)

    def select_rows(self,row_list,clearfirst=0,expose=1):
        """ Trigger row selection in the grid.

          Parameters:
              row_list- list of integers corresponding
                        to source (not grid or subset) index
                        coordinate to select

              clearfirst- 0 (don't clear existing selections before
                          selecting new ones) or 1 (clear exisiting
                          selections).  Defaults to 0.

              expose- 0 (redraw grid), or 1 (do not redraw grid).
                      Defaults to 1. 
        """

        self._flags['selecting-rows']=1
        if self.src_type == SRC_SHAPESLAYER:
            # block so that grid doesn't refresh until the end.
            self.layer.handler_block(self.layer_selection_changed_id)
            self.layer.handler_block(self.layer_subselection_changed_id)
            if clearfirst == 1:
                self.layer.clear_selection()
            for idx in range(len(row_list)-1):
                self.layer.select_shape(row_list[idx])
            self.layer.handler_unblock(
                             self.layer_selection_changed_id)
            self.layer.handler_unblock(
                             self.layer_subselection_changed_id)
            # The layer's selection-changed signal should
            # trigger a callback that calls _select_rows
            if len(row_list) > 0:
                self.layer.select_shape(row_list[len(row_list)-1]) 
        else:
            if clearfirst == 1:
                self._unselect_all_rows()
            self._select_rows(row_list)

        self._flags['selecting-rows']=0

        if expose == 1:

            nlist=self._rows_updated()

            if len(row_list) > 0:
                self.reset_startrow(row_list[len(row_list)-1])

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1


    def unselect_rows(self,row_list,clearfirst=0,expose=1):
        """ Trigger row selection in the grid.

          Parameters:
              row_list- list of integers corresponding
                        to source (not grid or subset) index
                        coordinate to select

              clearfirst- 0 (don't alter existing selections before
                          unselecting) or 1 (clear all values to
                          selected before unselecting requested
                          rows). Defaults to 0.

              expose- 0 (redraw grid), or 1 (do not redraw grid).
                      Defaults to 1. 
        """

        self._flags['selecting-rows']=1
        if self.src_type == SRC_SHAPESLAYER:
            # block so that grid doesn't refresh until the end.
            self.layer.handler_block(self.layer_selection_changed_id)
            self.layer.handler_block(self.layer_subselection_changed_id)
            if clearfirst == 1:
                self.layer.select_all()
            for idx in range(len(row_list)-1):
                self.layer.deselect_shape(row_list[idx])
            self.layer.handler_unblock(
                             self.layer_selection_changed_id)
            self.layer.handler_unblock(
                             self.layer_subselection_changed_id)
            # The layer's selection-changed signal should
            # trigger a callback that calls _select_rows
            if len(row_list) > 0:
                self.layer.deselect_shape(row_list[len(row_list)-1])
        else:
            if clearfirst == 1:
                self._select_all_rows()

            self._unselect_rows(row_list)

        self._flags['selecting-rows']=0

        if len(row_list) > 0:
            self.last_toggled_row=row_list[len(row_list)-1]

        if expose == 1: 

            nlist=self._rows_updated()

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1

    def select_row(self,row,clearfirst=0,expose=1):
        """Select a single row."""

        self.select_rows([row],clearfirst,expose)

    def unselect_row(self,row,clearfirst=0,expose=1):
        """Unselect a single row."""

        self.unselect_rows([row],clearfirst,expose)

    def toggle_row(self,row,expose=1):
        """Toggle a single row."""

        if self.row_selectstate[row] == 0:
            self.select_row(row,0,expose)
        else:
            self.unselect_row(row,0,expose)

    def toggle_rows(self,row_list,expose=1):
        """Toggle multiple rows."""

        select=[]
        unselect=[]

        for row in row_list:
            if self.row_selectstate[row] == 0:
                select.append(row)
            else:
                unselect.append(row)

        if len(unselect) > 0:
            self.unselect_rows(unselect,0,0)

        if len(select) > 0:
            self.select_rows(select,0,0)

        if expose == 1:

            nlist=self._rows_updated()

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def toggle_block_rows(self,end_row,expose=1):
        """Toggle a block of rows between self.last_toggled_row
           and end_row.

           Parameters:
               end_row- last row to include in toggle block

               expose- 0 (don't redraw grid) or 1 (redraw grid).
                       Defaults to 1.

        """

        if self.last_toggled_row is None:
            return

        g1=self.src2row[self.last_toggled_row]
        g2=self.src2row[end_row]
        if g1 < g2:
            tlist=list(self.row2src[g1+1:g2+1])
        else:
            tlist=list(self.row2src[g2:g1])

        self.toggle_rows(tlist,expose=0)
        self.last_toggled_row=end_row
        if self.row_selectstate[end_row] == 1:
            self.last_selected_row=end_row

        if expose == 1:

            nlist=self._rows_updated()

            self.reset_startrow(end_row)

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def select_block_rows(self,end_row,expose=1):
        """Select a block of rows between self.last_toggled_row
           and end_row.

           Parameters:
               end_row- last row to include in toggle block

               expose- 0 (don't redraw grid) or 1 (redraw grid).
                       Defaults to 1.

        """

        if self.last_toggled_row is None:
            return

        g1=self.src2row[self.last_toggled_row]
        g2=self.src2row[end_row]
        if g1 < g2:
            tlist=list(self.row2src[g1+1:g2+1])
        else:
            tlist=list(self.row2src[g2:g1])

        self.select_rows(tlist,expose=0)
        self.last_toggled_row=end_row
        self.last_selected_row=end_row

        if expose == 1:

            nlist=self._rows_updated()

            self.reset_startrow(end_row)

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def select_all_rows(self,expose=1):
        """Select all rows (updates internal matrices,
           triggers layer selection if relevant,
           and redraws grid if requested).
        """

        # TO DO: ADD CODE TO CHECK PGUGRID OPTIONS
        # AND MAKE SURE NO ILLEGAL SELECTION
        # IS PERMITTED (RAISE ERROR IF IT TRIES)

        self._flags['selecting-rows']=1
        if self.src_type == SRC_SHAPESLAYER:
            self.layer.select_all()
        else:
            self._select_all_rows()

        self._flags['selecting-rows']=0

        self.last_toggled_row=None
        self.last_selected_row=None

        if expose == 1:

            nlist=self._rows_updated()

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def unselect_all_rows(self,expose=1):
        """Unselect all rows (updates internal matrices,
           triggers layer selection if relevant,
           and redraws grid if requested).
        """

        self._flags['selecting-rows']=1
        if self.src_type == SRC_SHAPESLAYER:
            self.layer.clear_selection()
        else:
            self._unselect_all_rows()

        self._flags['selecting-rows']=0

        self.last_toggled_row=None
        self.last_selected_row=None

        if expose == 1:

            nlist=self._rows_updated()

            self.expose()

            if self.src_type == SRC_SHAPESLAYER:
                self.layer.display_change()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def _select_all_rows(self):
        """ Update internal selection matrices. """
        if self.row_selectstate is None:
            return 0

        self.row_selectstate[:] = 1
        self.selected_rows=range(len(self.row_selectstate))

        # check for cross-selection
        if self.opts.selection_info.row_cell == 0:
            if len(self.selected_cells) > 0:
                self.unselect_all_cells(expose=0)
                Signaler.notify(self, 'cell-selection-changed',())

        if self.opts.selection_info.row_column == 0:
            if len(self.selected_columns) > 0:
                self.unselect_all_columns(expose=0)
                Signaler.notify(self, 'column-selection-changed',())


        return 1

    def _unselect_all_rows(self):
        """ Update internal selection matrices. """
        if self.row_selectstate is None:
            return 0

        self.row_selectstate[:] = 0
        self.selected_rows=[]

        return 1

    def _select_rows(self,row_list):
        """ Helper function for select_rows.  Updates
            the selection matrices internally.  
        """
        if self.row_selectstate is None:
            # No source is set
            return 0

        for row in row_list:
            if (len(self.row_selectstate) <= row):
                raise RuntimeError,'pgugrid: tried to select nonexistent row'
            else:
                if self.row_selectstate[row] == 0:
                    self.row_selectstate[row]=1
                    self.selected_rows.append(row)


        if len(row_list) > 0:
            self.last_selected_row=row_list[len(row_list)-1]
            self.last_toggled_row=row_list[len(row_list)-1]

            # check for cross-selection
            if self.opts.selection_info.row_cell == 0:
                if len(self.selected_cells) > 0:
                    self.unselect_all_cells(expose=0)
                    Signaler.notify(self, 'cell-selection-changed',())
            elif self.opts.selection_info.row_cell == 1:
                if len(self.selected_cells) > 0:
                    uscells=[]
                    for item in self.selected_cells:
                        if self.row_selectstate[item[0]] == 0:
                            uscells.append(item)
                    if len(uscells) > 0:          
                        self.unselect_cells(uscells,expose=0)
                        Signaler.notify(self, 'cell-selection-changed',
                                    tuple(self.selected_cells))

            if self.opts.selection_info.row_column == 0:
                if len(self.selected_columns) > 0:
                    self.unselect_all_columns(expose=0)
                    Signaler.notify(self, 'column-selection-changed',())

        return 1

    def _unselect_rows(self,row_list):
        """ Helper function for unselect_rows.  Updates
            the selection matrices internally.  
        """
        if self.row_selectstate is None:
            # No source is set
            return 0

        for row in row_list:
            if (len(self.row_selectstate) <= row):
                raise RuntimeError,'pgugrid: tried to select nonexistent row'
            else:
                if self.row_selectstate[row] == 1:
                    self.row_selectstate[row]=0
                    self.selected_rows.remove(row)

        return 1

    def select_column(self,column,clearfirst=0,expose=1):
        """Select a single column."""

        rval = self.select_columns([column],clearfirst,expose)
        return rval

    def select_columns(self,column_list,clearfirst=0,expose=1):
        """Select multiple columns."""

        if self.column_selectstate is None:
            # No source is set
            return 0

        if clearfirst == 1:
            self.unselect_all_columns(expose=0)

        for column in column_list:
            if (len(self.column_selectstate) <= column):
                raise RuntimeError,'pgugrid: tried to select nonexistent '+\
                      'column'
            else:
                self.column_selectstate[column]=1

            if column not in self.selected_columns:
                self.selected_columns.append(column)


        if len(column_list) > 0:
            self.last_selected_column=column_list[len(column_list)-1]
            self.last_toggled_column=column_list[len(column_list)-1]


        if expose == 1:

            nlist=self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1

    def select_all_columns(self,expose=1):
        """Select all columns."""

        if self.column_selectstate is None:
            return 0

        self.column_selectstate[:]=1
        self.selected_columns=range(len(self.column_selectstate))

        self.last_toggled_column=None
        self.last_selected_column=None

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1

    def unselect_column(self,column,clearfirst=0,expose=1):
        """Unselect a single column."""

        rval = self.unselect_columns([column],clearfirst,expose)
        return rval


    def unselect_columns(self,column_list,clearfirst=0,expose=1):
        """Unselect multiple columns."""

        if self.column_selectstate is None:
            # No source is set
            return 0

        if clearfirst == 1:
            self.select_all_columns(expose=0)

        for column in column_list:
            if (len(self.column_selectstate) <= column):
                raise RuntimeError,'pgugrid: tried to select nonexistent '+\
                      'column'
            else:
                self.column_selectstate[column]=0

            if column in self.selected_columns:
                self.selected_columns.remove(column)

        if len(column_list) > 0:
            self.last_toggled_column=column_list[len(column_list)-1]

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1        

    def unselect_all_columns(self,expose=1):
        """Unselect all columns."""

        if self.column_selectstate is None:
            return 0

        self.column_selectstate[:]=0
        self.selected_columns=[]

        self.last_toggled_column=None
        self.last_selected_column=None

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1


    def toggle_column(self,column,expose=1):
        """Toggle a single column."""

        if self.column_selectstate[column] == 0:
            self.select_column(column,0,expose)
        else:
            self.unselect_column(column,0,expose)


    def toggle_columns(self,column_list,expose=1):
        """Toggle multiple columns."""

        select=[]
        unselect=[]

        for column in column_list:
            if self.column_selectstate[column] == 0:
                select.append(column)
            else:
                unselect.append(column)

        if len(unselect) > 0:
            self.unselect_columns(unselect,0,0)

        if len(select) > 0:
            self.select_columns(select,0,0)

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])


    def toggle_block_columns(self,end_column,expose=1):
        """Toggle a block of columns between self.last_toggled_column
           and end_column.

           Parameters:
               end_column- last column to include in toggle block

               expose- 0 (don't redraw grid) or 1 (redraw grid).
                       Defaults to 1.

        """

        if self.last_toggled_column is None:
            return

        if self.last_toggled_column < end_column:
            tlist=range(self.last_toggled_column+1,end_column+1)
        else:
            tlist=range(end_column,self.last_toggled_column)

        self.toggle_columns(tlist,expose=0)
        self.last_toggled_column=end_column
        if self.column_selectstate[end_column] == 1:
            self.last_selected_column=end_column

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])


    def select_block_columns(self,end_column,expose=1):
        """Select a block of columns between self.last_toggled_column
           and end_column.

           Parameters:
               end_column- last column to include in toggle block

               expose- 0 (don't redraw grid) or 1 (redraw grid).
                       Defaults to 1.

        """

        if self.last_toggled_column is None:
            return

        if self.last_toggled_column < end_column:
            tlist=range(self.last_toggled_column+1,end_column+1)
        else:
            tlist=range(end_column,self.last_toggled_column)

        self.select_columns(tlist,expose=0)
        self.last_toggled_column=end_column
        self.last_selected_column=end_column

        if expose == 1:

            nlist = self._columns_updated()

            self.expose()

            Signaler.notify(self, 'column-selection-changed',
                        tuple(self.selected_columns))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def select_cell(self,cell,clearfirst=0,expose=1):
        """Select a single cell."""
        rval = self.select_cells([cell],clearfirst,expose=expose)
        return rval


    def select_cells(self,cell_list,clearfirst=0,expose=1):
        """Select multiple cells.  cell_list is a list of
           (source row, grid column) tuples.
        """

        if self.cell_selectstate is None:
            # No source is set
            return 0

        if clearfirst == 1:
            self.unselect_all_cells(expose=0)

        for cell in cell_list:
            if ((self.cell_selectstate.shape[0] <= cell[0]) or
                (self.cell_selectstate.shape[1] <= cell[1])):
                raise RuntimeError,'pgugrid: tried to select nonexistent '+\
                      'cell'
            else:
                if self.cell_selectstate[cell[0],cell[1]] == 0:
                    self.cell_selectstate[cell[0],cell[1]]=1
                    self.selected_cells.append(cell)


        if len(cell_list) > 0:
            self.last_selected_cell=cell_list[len(cell_list)-1]
            self.last_toggled_cell=cell_list[len(cell_list)-1]


        if expose == 1:
            nlist=self._cells_updated()

            self.expose()

            Signaler.notify(self, 'cell-selection-changed',
                        tuple(self.selected_cells))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1


    def select_all_cells(self,expose=1):
        """Select all cells.
           NOT USED YET- MAY NOT EVER BE.
        """

        if self.cell_selectstate is None:
            return 0

        self.cell_selectstate[:,:]=1
        # NOTE: it would be more efficient to implement
        # selected cells so that it could be a tuple of
        # slices or a tuple of indices, but this is
        # simpler to implement (the indices method, that
        # is).  This also applies to row and column
        # selection.  May want to revisit later.  
        self.selected_cells = []

        for i in range(self.cell_selectstate.shape[0]):
            for j in range(self.cell_selectstate.shape[1]):
                self.selected_cells.append((i,j))


        self.last_toggled_cell=None
        self.last_selected_cell=None

        if expose == 1:

            nlist=self._cells_updated()

            self.expose()

            Signaler.notify(self, 'cell-selection-changed',
                        tuple(self.selected_cells))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])


        return 1


    def unselect_cell(self,cell,clearfirst=0,expose=1):
        """Unselect a single cell."""

        rval = self.unselect_cells([cell],clearfirst,expose)
        return rval


    def unselect_cells(self,cell_list,clearfirst=0,expose=1):
        """Unselect multiple cells.  cell_list is a list of
           (source row, grid column) tuples.
        """

        if self.cell_selectstate is None:
            # No source is set
            return 0

        if clearfirst == 1:
            self.select_all_cells(expose=0)

        for cell in cell_list:
            if ((self.cell_selectstate.shape[0] <= cell[0]) or
                (self.cell_selectstate.shape[1] <= cell[1])):
                raise RuntimeError,'pgugrid: tried to select nonexistent '+\
                      'cell'
            else:
                if self.cell_selectstate[cell[0],cell[1]] == 1:
                    self.cell_selectstate[cell[0],cell[1]]=0
                    self.selected_cells.remove(cell)

        # TO DO: check this!!!
        if len(cell_list) > 0:
            self.last_toggled_cell=cell_list[len(cell_list)-1]


        if expose == 1:

            nlist = self._cells_updated()

            self.expose()

            Signaler.notify(self, 'cell-selection-changed',
                        tuple(self.selected_cells))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1


    def unselect_all_cells(self,expose=1):
        """Unselect all cells."""

        if self.cell_selectstate is None:
            return 0

        self.cell_selectstate[:,:]=0
        self.selected_cells = []

        self.last_toggled_cell=None
        self.last_selected_cell=None

        if expose == 1:

            nlist=self._cells_updated()

            self.expose()

            Signaler.notify(self, 'cell-selection-changed',
                        tuple(self.selected_cells))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

        return 1


    def toggle_cell(self,cell,expose=1):
        """Toggle a single cell."""

        if self.cell_selectstate[cell[0],cell[1]] == 0:
            self.select_cell(cell,0,expose)
        else:
            self.unselect_cell(cell,0,expose)


    def toggle_cells(self,cell_list,expose=1):
        """Toggle multiple cells."""

        select=[]
        unselect=[]

        for cell in cell_list:
            if self.cell_selectstate[cell] == 0:
                select.append(cell)
            else:
                unselect.append(cell)

        if len(unselect) > 0:
            self.unselect_cells(unselect,0,0)

        if len(select) > 0:
            self.select_cells(select,0,0)

        if expose == 1:

            nlist = self._cells_updated()     

            self.expose()

            Signaler.notify(self, 'cell-selection-changed',
                        tuple(self.selected_cells))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])



    def toggle_block_cells(self,end_cell,expose=1):
        """Toggle a block of cells between self.last_toggled_cell
           and end_cell.

           Parameters:
               end_cell- last cell to include in toggle block,
                         a (row,column) tuple.

               expose- 0 (don't redraw grid) or 1 (redraw grid).
                       Defaults to 1.

        """

        if self.last_toggled_cell is None:
            return

        g1=self.src2row[self.last_toggled_cell[0]]
        g2=self.src2row[end_cell[0]]
        if g1 < g2:
            rlist=list(self.row2src[g1:g2+1])
        else:
            rlist=list(self.row2src[g2:g1+1])

        if self.last_toggled_cell[1] < end_cell[1]:
            clist=range(self.last_toggled_cell[1],end_cell[1]+1)
        else:
            clist=range(end_cell[1],self.last_toggled_cell[1]+1)

        tlist=[]
        for row in rlist:
            for column in clist:
                ncell=(row,column)
                if ncell != self.last_toggled_cell:
                    tlist.append(ncell)

        self.toggle_cells(tlist,expose=expose)

        self.last_toggled_cell=end_cell
        if self.cell_selectstate[end_cell[0],end_cell[1]] == 1:
            self.last_selected_cell=end_cell

    def _cells_updated(self):
        """ Internal function called when cell selection has
            changed.  Ensures that row/column selections is
            consistent with current cross-selection settings.
            Returns notifications that should be sent out
            after expose event.
        """

        notifylist=[]
        if len(self.selected_cells) > 0:
            if self.opts.selection_info.row_cell == 0:
                if len(self.selected_rows) > 0:
                    self.unselect_all_rows(expose=0)
                    if self.src_type == SRC_SHAPESLAYER:
                        self.layer.display_change()
                    notifylist.append(('row-selection-changed',
                            tuple(self.selected_rows)))
            elif self.opts.selection_info.row_cell == 1:
                # find rows that have selected cells in them,
                # unselect any selected rows that don't have
                # selected cells in them
                rstate=Numeric.where(Numeric.sum(self.cell_selectstate,1)>0,
                                     1,0)
                unselect=Numeric.where(
                    self.row_selectstate*2+rstate == 2,
                    Numeric.arange(self.s_rows),-1)
                ulist=list(Numeric.compress(unselect > -1, unselect))
                if len(ulist) > 0:
                    self.unselect_rows(ulist,expose=0)
                    if self.src_type == SRC_SHAPESLAYER:
                        self.layer.display_change()    
                    notifylist.append(('row-selection-changed',
                                tuple(self.selected_rows)))
            elif self.opts.selection_info.row_cell == 3:
                # rows that contain selected cells should be
                # selected; those that don't should not be
                # selected
                rstate=Numeric.where(Numeric.sum(self.cell_selectstate,1)>0,
                                     1,0)
                unselect=Numeric.where(
                    self.row_selectstate*2+rstate == 2,
                    Numeric.arange(self.s_rows),-1)
                ulist=list(Numeric.compress(unselect > -1, unselect))
                if len(ulist) > 0:
                    self.unselect_rows(ulist,expose=0)

                select=Numeric.where(
                    self.row_selectstate*2+rstate == 1,
                    Numeric.arange(self.s_rows),-1)
                slist=list(Numeric.compress(select > -1, select))
                if len(slist) > 0:
                    self.select_rows(slist,expose=0)

                if ((len(ulist) > 0) or (len(slist) > 0)): 
                    if self.src_type == SRC_SHAPESLAYER:
                        self.layer.display_change()   
                    notifylist.append(('row-selection-changed',
                                tuple(self.selected_rows)))

            if self.opts.selection_info.column_cell == 0:
                if len(self.selected_columns) > 0:
                    self.unselect_all_columns(expose=0)
                    notifylist.append(('column-selection-changed',
                            tuple(self.selected_columns)))                
            elif self.opts.selection_info.column_cell == 1:
                cstate=Numeric.where(Numeric.sum(self.cell_selectstate,0)>0,
                                     1,0)
                unselect=Numeric.where(
                    self.column_selectstate*2+cstate == 2,
                    Numeric.arange(self.g_columns),-1)
                ulist=list(Numeric.compress(unselect > -1, unselect))
                if len(ulist) > 0:
                    self.unselect_columns(ulist,expose=0)
                    notifylist.append(('column-selection-changed',
                                tuple(self.selected_columns)))                
            elif self.opts.selection_info.column_cell == 3:
                cstate=Numeric.where(Numeric.sum(self.cell_selectstate,0)>0,
                                     1,0)
                unselect=Numeric.where(
                    self.column_selectstate*2+cstate == 2,
                    Numeric.arange(self.g_columns),-1)
                ulist=list(Numeric.compress(unselect > -1, unselect))
                if len(ulist) > 0:
                    self.unselect_columns(ulist,expose=0)

                select=Numeric.where(
                    self.column_selectstate*2+cstate == 1,
                    Numeric.arange(self.g_columns),-1)
                slist=list(Numeric.compress(select > -1, select))
                if len(slist) > 0:
                    self.select_columns(slist,expose=0)

                if ((len(ulist) > 0) or (len(slist) > 0)): 
                    notifylist.append(('column-selection-changed',
                                tuple(self.selected_columns)))

        else:
            if self.opts.selection_info.row_cell == 3:
                if len(self.selected_rows) > 0:
                    self.unselect_all_rows(expose=0)
                    if self.src_type == SRC_SHAPESLAYER:
                        self.layer.display_change()
                    notifylist.append(('row-selection-changed',()))

            if self.opts.selection_info.column_cell == 3:
                if len(self.selected_columns) > 0:
                    self.unselect_all_columns(expose=0)
                    notifylist.append(('column-selection-changed',()))

        if ((self.editing_cell is not None) and
            (self.cell_selectstate[self.editing_cell[0],
             self.editing_cell[1]] == 0)):
            self.end_cell_edit()

        return notifylist

    def _rows_updated(self):
        """ Internal function called when row selection has
            changed.  Ensures that cell/column selections is
            consistent with current cross-selection settings.
            Returns a list of notifications to send out.
        """

        notifylist=[]       
        if self.g_columns == 0:
            # No columns are defined yet, so
            # there must be no column/cell selections
            return notifylist

        if len(self.selected_rows) > 0:
            if self.opts.selection_info.row_cell == 0:
                if len(self.selected_cells) > 0:
                    self.unselect_all_cells(expose=0)                 
                    notifylist.append(('cell-selection-changed',
                            tuple(self.selected_cells)))
            elif self.opts.selection_info.row_cell == 1:
                # unselect any selected cells that aren't contained
                # in selected rows
                rvec=Numeric.repeat(Numeric.reshape(self.row_selectstate,
                             (self.s_rows,1)),self.g_columns,1)

                unselect=Numeric.reshape(Numeric.where(
                    self.cell_selectstate > rvec,1,0),
                    (self.s_rows*self.g_columns,))
                cind=Numeric.indices((self.s_rows,self.g_columns))
                rarr=Numeric.reshape(cind[0],(self.s_rows*self.g_columns,))
                carr=Numeric.reshape(cind[1],(self.s_rows*self.g_columns,))

                rcarr=Numeric.compress(unselect > 0,rarr)
                ccarr=Numeric.compress(unselect > 0,carr)
                uarr=Numeric.zeros((rcarr.shape[0],2))
                uarr[:,0]=rcarr
                uarr[:,1]=ccarr

                ulist=map(tuple,uarr)
                if len(ulist) > 0:
                    self.unselect_cells(ulist,expose=0)
                    notifylist.append(('cell-selection-changed',
                                tuple(self.selected_cells)))

            elif self.opts.selection_info.row_cell == 3:
                # unselect any selected cells that aren't contained
                # in selected rows.  Force selection of first cell
                # in any row that doesn't contain any selected cells
                # (ensures that row can be unselected again through
                # grid in shapeslayer case if no row titles are shown)

                rvec=Numeric.repeat(Numeric.reshape(self.row_selectstate,
                             (self.s_rows,1)),self.g_columns,1)

                unselect=Numeric.reshape(Numeric.where(
                    self.cell_selectstate > rvec,1,0),
                    (self.s_rows*self.g_columns,))
                cind=Numeric.indices((self.s_rows,self.g_columns))
                urarr=Numeric.reshape(cind[0],(self.s_rows*self.g_columns,))
                ucarr=Numeric.reshape(cind[1],(self.s_rows*self.g_columns,))

                urcarr=Numeric.compress(unselect > 0,urarr)
                uccarr=Numeric.compress(unselect > 0,ucarr)
                uarr=Numeric.zeros((urcarr.shape[0],2))
                uarr[:,0]=urcarr
                uarr[:,1]=uccarr

                ulist=map(tuple,uarr)
                if len(ulist) > 0:
                    self.unselect_cells(ulist,expose=0) 

                sumc=Numeric.sum(self.cell_selectstate,1)
                select=Numeric.where(sumc < self.row_selectstate,
                                     Numeric.arange(self.s_rows),-1)
                srcarr=Numeric.compress(select > -1,select)
                sarr=Numeric.zeros((srcarr.shape[0],2))
                sarr[:,0]=srcarr
                sarr[:,1]=Numeric.zeros((srcarr.shape[0],))

                slist=map(tuple,sarr)
                if len(slist) > 0:
                    self.select_cells(slist,expose=0)

                if (len(ulist) > 0) or (len(slist) > 0):                    
                    notifylist.append(('cell-selection-changed',
                                tuple(self.selected_cells)))

            if self.opts.selection_info.row_column == 0:
                if len(self.selected_columns) > 0:
                    self.unselect_all_columns(expose=0)
                    notifylist.append(('column-selection-changed',
                            tuple()) )

        else:
            if self.opts.selection_info.row_cell == 3:
                if len(self.selected_cells) > 0:
                    self.unselect_all_cells(expose=0) 
                    notifylist.append(('cell-selection-changed',()))

        # If row selection changes while a cell is being edited,
        # cancel editing and clear cell selections.
        if (self.editing_cell is not None):
            self.cancel_cell_edit()
            self.unselect_all_cells()

        return notifylist


    def _columns_updated(self):
        """ Internal function called when column selection has
            changed.  Ensures that row/cell selections is
            consistent with current cross-selection settings.
            Returns a list of notifications to send out.
        """

        notifylist=[]
        if len(self.selected_columns) > 0:
            if self.opts.selection_info.column_cell == 0:
                if len(self.selected_cells) > 0:
                    self.unselect_all_cells(expose=0)
                    notifylist.append(('cell-selection-changed',
                            tuple(self.selected_cells)))
            elif self.opts.selection_info.column_cell == 1:
                # unselect any selected cells that aren't contained
                # in selected columns
                if self.s_rows > 0:
                    rvec=Numeric.repeat(Numeric.reshape(
                             self.column_selectstate,
                             (1,self.g_columns)),self.s_rows,0)

                    unselect=Numeric.reshape(Numeric.where(
                        self.cell_selectstate > rvec,1,0),
                        (self.s_rows*self.g_columns,))
                    cind=Numeric.indices((self.s_rows,self.g_columns))
                    rarr=Numeric.reshape(cind[0],(self.s_rows*self.g_columns,))
                    carr=Numeric.reshape(cind[1],(self.s_rows*self.g_columns,))

                    rcarr=Numeric.compress(unselect > 0,rarr)
                    ccarr=Numeric.compress(unselect > 0,carr)
                    uarr=Numeric.zeros((rcarr.shape[0],2))
                    uarr[:,0]=rcarr
                    uarr[:,1]=ccarr

                    ulist=map(tuple,uarr)
                    if len(ulist) > 0:
                        self.unselect_cells(ulist,expose=0)
                        notifylist.append(('cell-selection-changed',
                                tuple(self.selected_cells)))

            elif self.opts.selection_info.column_cell == 3:
                # unselect any selected cells that aren't contained
                # in selected columns.  Force selection of first cell
                # in any column that doesn't contain any selected cells
                # (ensures that column can be unselected again through
                # grid in shapeslayer case if no column titles are shown)

                if self.s_rows > 0:                
                    rvec=Numeric.repeat(Numeric.reshape(
                             self.column_selectstate,
                             (1,self.g_columns)),self.s_rows,0)

                    unselect=Numeric.reshape(Numeric.where(
                        self.cell_selectstate > rvec,1,0),
                        (self.s_rows*self.g_columns,))
                    cind=Numeric.indices((self.s_rows,self.g_columns))
                    urarr=Numeric.reshape(cind[0],
                                          (self.s_rows*self.g_columns,))
                    ucarr=Numeric.reshape(cind[1],
                                          (self.s_rows*self.g_columns,))

                    urcarr=Numeric.compress(unselect > 0,urarr)
                    uccarr=Numeric.compress(unselect > 0,ucarr)
                    uarr=Numeric.zeros((urcarr.shape[0],2))
                    uarr[:,0]=urcarr
                    uarr[:,1]=uccarr

                    ulist=map(tuple,uarr)
                    if len(ulist) > 0:
                        self.unselect_cells(ulist,expose=0)

                    sumc=Numeric.sum(self.cell_selectstate,1)
                    select=Numeric.where(sumc < self.row_selectstate,
                                     Numeric.arange(self.s_rows),-1)
                    srcarr=Numeric.compress(select > -1,select)
                    sarr=Numeric.zeros((srcarr.shape[0],2))
                    sarr[:,0]=srcarr
                    sarr[:,1]=Numeric.zeros((srcarr.shape[0],))

                    slist=map(tuple,sarr)
                    if len(slist) > 0:
                        self.select_cells(slist,expose=0)

                    if (len(ulist) > 0) or (len(slist) > 0):           
                        notifylist.append(('cell-selection-changed',
                                tuple(self.selected_cells)))

            if self.opts.selection_info.row_column == 0:
                if len(self.selected_rows) > 0:
                    self.unselect_all_rows(expose=0)
                    if self.src_type == SRC_SHAPESLAYER:
                        self.layer.display_change()
                    notifylist.append(('row-selection-changed',
                            tuple()) )

        else:
            if self.opts.selection_info.column_cell == 3:
                if len(self.selected_cells) > 0:
                    self.unselect_all_cells(expose=0) 
                    notifylist.append(('cell-selection-changed',()))



        if ((self.editing_cell is not None) and
            (self.cell_selectstate[self.editing_cell[0],
                                   self.editing_cell[1]] == 0)):
            self.end_cell_edit()

        return notifylist


    def _update_column_width(self,column,cell_width):
        """Reset a column's width, and update other
           columns' start positions accordingly.
        """

        self._ColumnDefs[column].width=cell_width

        startx=self._ColumnDefs[column].start_x +\
               self._ColumnDefs[column].width + 1

        for idx in range(column+1,self.g_columns):
            self._ColumnDefs[idx].start_x=startx
            startx=startx+self._ColumnDefs[idx].width+1

    def reset_startrow(self, row):
        """ Check if row is visible.  If it is,
            return without doing anything.  If it
            isn't, update the GUI so that it
            is visible. row should be in source
            coordinates.  Does nothing if row
            is None or isn't in the current subset.
        """
        if not (self.flags() & gtk.REALIZED):
            return

        c_row = self.src2row[row]
        if c_row < 0:
            # requested row is either None, or is not
            # part of the currently displayed subset.
            return

        win = self._area.window
        width, height = win.get_size()

        base_height = height
        column_title_height = self.column_title_height

        # The extra 1's account for the 1-pixel wide lines drawn
        # between cells.
        data_height = base_height - column_title_height -1
        if self.opts.show_column_titles == 1:
            data_height=data_height - 1

        disp_rows = int(data_height / ( self.row_height + 1 )) + 1
        first_row = self.start_row
        last_row = first_row + disp_rows - 1

        if ((c_row >= self.start_row) and (c_row < last_row)):
            return

        # If c_row isn't in current window, scroll
        # so it is the new start row.
        vmax = self.vadj.__getattr__('upper')
        if c_row >= vmax:
            self.vadj.set_value(vmax)
        elif c_row < 0:
            self.vadj.set_value(0)
        else:
            self.vadj.set_value(c_row)

        self.vadj.value_changed()


    def reset_startcolumn(self, column):
        """ Check if column is visible.  If it is,
            return without doing anything.  If it
            isn't, update the GUI so that it
            is visible. Does nothing if column
            is outside the range of columns.
        """
        if not (self.flags() & gtk.REALIZED):
            return


        win = self._area.window
        width, height = win.get_size()

        base_width = width
        row_title_width = self.row_title_width

        # The extra 1's account for the 1-pixel wide lines drawn
        # between cells.
        data_width = base_width - row_title_width -1
        if self.opts.show_row_titles == 1:
            data_width=data_width - 1

        cstartx = self._ColumnDefs[self.start_column].start_x
        cendx=cstartx+data_width

        ccolumn=column
        if ccolumn < 0:
            ccolumn=0
        elif ccolumn >= self.g_columns:
            ccolumn=self.g_columns-1

        nstartx=self._ColumnDefs[ccolumn].start_x
        nendx=nstartx+self._ColumnDefs[ccolumn].width

        if ((nstartx >= cstartx) and (nendx < cendx)):
            return

        # If ccolumn isn't in current window, scroll
        # so it is the new start column.
        self.hadj.set_value(ccolumn)
        self.hadj.value_changed()


    def click( self, widget, event ):
        """ User has clicked on the widget.
        """

        if len(self.column_widths) < 1:
            return

        self._area.grab_focus()
        #
        # determine the column that the user clicked in by first offsetting
        # for the current start column (accounts for columns scrolled off the
        # left edge).
        #
        # After this if-block, nColumn should be -1 if a row title
        # was clicked, None if the user clicked off the right
        # edge of the grid, and the column number if a user
        # clicked on a cell.
        #

        if ((self.opts.show_row_titles == 1) and
           (event.x < self.row_title_width+2)):
            # clicked on row title column
            gColumn = -1
        else:
            #
            # now actually look for the right column.
            # If gColumn is None at the end
            # then the user clicked off the right edge.
            #
            gColumn = None
            current = 1
            if self.opts.show_row_titles == 1:
                current = self.row_title_width+2

            for i in range(self.start_column,len(self.column_widths)):
                current = current + self.column_widths[i] + 1
                if event.x < current:
                    gColumn = i
                    break


        #
        # Determine the row.  If its -1 then they clicked a 'title'.  If it
        # is None then they clicked off the bottom edge.
        #
        gRow = None
        if ((self.opts.show_column_titles == 1) and
            (event.y < (self.column_title_height + 2))):
            gRow = -1
        else:
            # NOTE: the max(self.start_row-1,0) below is a kludge to
            # avoid an offset problem after scrolling (first scroll
            # click doesn't seem to actually cause the window to scroll,
            # even though vadj updates)
            current=1
            if self.opts.show_column_titles == 1:
                current = self.column_title_height+2

            row = self.start_row+\
                  int(Numeric.floor((event.y-current) /(self.row_height + 1)))

            if row < self.g_rows:
                gRow = row

        if ((gColumn is None) or (gRow is None) or
            ((gColumn == -1) and (gRow == -1))):
            # User did not click on a cell or
            # row/column header
            return

        if gRow != -1:
            sRow=self.row2src[gRow]
        else:
            sRow = -1

        clickstr=None
        clickarg=None

        if (event.state & gtk.gdk.SHIFT_MASK):
            # last click information only applies
            # if the click was not modified by
            # shift or control (for the purposes
            # of pgugrid).  Last click is not
            # related to last selected.
            self.last_click_cell=None
            self.last_click_row=None
            self.last_click_column=None
            self.last_click_button=None

            if event.button == 1:
                if gRow == -1:
                    # A column header was clicked
                    clickstr='column-shift-left'
                    clickarg=(gColumn,)
                elif gColumn == -1:
                    clickstr='row-shift-left'
                    clickarg=(sRow,)
                else:
                    clickstr='cell-shift-left'
                    clickarg=(sRow,gColumn)
            elif event.button == 3:
                if gRow == -1:
                    # A column header was clicked
                    clickstr='column-shift-right'
                    clickarg=(gColumn,)
                elif gColumn == -1:
                    clickstr='row-shift-right'
                    clickarg=(sRow,)
                else:
                    clickstr='cell-shift-right'
                    clickarg=(sRow,gColumn)

        elif (event.state & gtk.gdk.CONTROL_MASK):
            self.last_click_cell=None
            self.last_click_row=None
            self.last_click_column=None
            self.last_click_button=None

            if event.button == 1:
                if gRow == -1:
                    # A column header was clicked
                    clickstr='column-ctrl-left'
                    clickarg=(gColumn,)
                elif gColumn == -1:
                    clickstr='row-ctrl-left'
                    clickarg=(sRow,)
                else:
                    clickstr='cell-ctrl-left'
                    clickarg=(sRow,gColumn)
            elif event.button == 3:
                if gRow == -1:
                    # A column header was clicked
                    clickstr='column-ctrl-right'
                    clickarg=(gColumn,)
                elif gColumn == -1:
                    clickstr='row-ctrl-right'
                    clickarg=(sRow,)
                else:
                    clickstr='cell-ctrl-right'
                    clickarg=(sRow,gColumn)
        else:
            # Current click event is not modified
            # by shift or control.  First, establish
            # if this is a double click event or not.

            if self.last_click_button == event.button:
                if event.button == 1:
                    if ((sRow == self.last_click_row) and
                        (gColumn == -1)):
                        clickstr='row-double-left'
                        clickarg=(sRow,)
                    elif ((gColumn == self.last_click_column) and
                          (gRow == -1)):
                        clickstr='column-double-left'
                        clickarg=(gColumn,)
                    elif ((sRow == self.last_click_cell[0]) and
                          (gColumn == self.last_click_cell[1])):
                        clickstr='cell-double-left'
                        clickarg=(sRow,gColumn)
                elif event.button == 3:
                    if ((sRow == self.last_click_row) and
                        (gColumn == -1)):
                        clickstr='row-double-right'
                        clickarg=(sRow,)
                    elif ((gColumn == self.last_click_column) and
                          (gRow == -1)):
                        clickstr='column-double-right'
                        clickarg=(gColumn,)
                    elif ((sRow == self.last_click_cell[0]) and
                          (gColumn == self.last_click_cell[1])):
                        clickstr='cell-double-right'
                        clickarg=(sRow,gColumn)

            if clickstr is not None:
                # a double click event has been detected.
                # clear the last_click values
                self.last_click_cell=(None,None)
                self.last_click_row=None
                self.last_click_column=None
                self.last_click_button=None
            else:
                self.last_click_cell=(None,None)
                self.last_click_row=None
                self.last_click_column=None
                self.last_click_button=event.button                
                if event.button == 1:
                    if gRow == -1:
                        # A column header was clicked
                        clickstr='column-left'
                        clickarg=(gColumn,)
                        self.last_click_column=gColumn
                    elif gColumn == -1:
                        clickstr='row-left'
                        clickarg=(sRow,)
                        self.last_click_row=sRow
                    else:
                        clickstr='cell-left'
                        clickarg=(sRow,gColumn)
                        self.last_click_cell=(sRow,gColumn)
                elif event.button == 3:
                    if gRow == -1:
                        # A column header was clicked
                        clickstr='column-right'
                        clickarg=(gColumn,)
                        self.last_click_column=gColumn
                    elif gColumn == -1:
                        clickstr='row-right'
                        clickarg=(sRow,)
                        self.last_click_row=sRow
                    else:
                        clickstr='cell-right'
                        clickarg=(sRow,gColumn)
                        self.last_click_cell=(sRow,gColumn)                


        if ((clickstr is not None) and
            (self.opts.events.has_key(clickstr))):
            # click event is recognized, and current
            # configuration has defined an event for it.
            self._perform_click_action(clickstr,clickarg)

        # Pass clicked event with grid row, column
        Signaler.notify(self, 'clicked',gRow,gColumn,event)



    def _perform_click_action(self,clickstr,clickarg):
        """Map the click event and arguments onto the
           correct function call for the current configuration.
        """
        # Find out what this action is supposed to do
        clickfunc=self.opts.events[clickstr]

        if clickfunc == 'select-single-row':
            # Select a single row at a time, clearing all
            # previous selections.
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.select_row(clickarg[0],clearfirst=1,expose=1)
        elif clickfunc == 'unselect-single-row':
            # Unselect the current row (and only that row)
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.unselect_row(clickarg[0],clearfirst=0,expose=1)
        elif clickfunc == 'toggle-single-row':
            # Toggle a single row, making sure all other rows
            # are unselected.
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            if self.row_selectstate[clickarg[0]] == 1:
                self.unselect_all_rows()
            else:
                self.unselect_all_rows(expose=0)
                self.select_row(clickarg[0])

        elif clickfunc == 'toggle-block-rows':
            # Toggle a block of rows between the last selected
            # row and the current one.
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.toggle_block_rows(clickarg[0],expose=1)             
        elif clickfunc == 'toggle-multiple-rows':
            # Toggle a single row without affecting other rows
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.toggle_row(clickarg[0],expose=1)

        elif clickfunc == 'select-block-rows':
            # Select a block of rows between the last selected
            # row and the current one.
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.select_block_rows(clickarg[0],expose=1)             
        elif clickfunc == 'select-multiple-rows':
            # Select a single row without affecting other rows
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.select_row(clickarg[0],expose=1)
        elif clickfunc == 'select-single-column':
            # Select a single column at a time, clearing all
            # previous selections.
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.select_column(clickarg[0],clearfirst=1,expose=1)
        elif clickfunc == 'unselect-single-column':
            # Unselect the current column
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.unselect_column(clickarg[0],clearfirst=0,expose=1)
        elif clickfunc == 'toggle-single-column':
            # Toggle a single column, making sure all other columns
            # are unselected.
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            if self.column_selectstate[clickarg[0]] == 1:
                self.unselect_all_columns()
            else:
                self.unselect_all_columns(expose=0)
                self.select_column(clickarg[0])

        elif clickfunc == 'toggle-block-columns':
            # Toggle a block of columns between the last selected
            # column and the current one.
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.toggle_block_columns(clickarg[0],expose=1)             

        elif clickfunc == 'toggle-multiple-columns':
            # Toggle a single column without affecting other columns
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.toggle_column(clickarg[0],expose=1)

        elif clickfunc == 'select-block-columns':
            # Select a block of columns between the last selected
            # column and the current one.
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.select_block_columns(clickarg[0],expose=1)             

        elif clickfunc == 'select-multiple-columns':
            # Toggle a single column without affecting other columns
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.select_column(clickarg[0],expose=1)

        elif clickfunc == 'select-single-cell':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt

            self.select_cell(clickarg,clearfirst=1,expose=1)

        elif clickfunc == 'unselect-single-cell':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt

            self.unselect_cell(clickarg,clearfirst=0,expose=1)

        elif clickfunc == 'toggle-single-cell':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt

            if self.cell_selectstate[clickarg[0],clickarg[1]] == 1:
                self.unselect_all_cells()
            else:
                self.unselect_all_cells(expose=0)
                self.select_cell(clickarg)

        elif clickfunc == 'toggle-block-cells':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt

            self.toggle_block_cells(clickarg,expose=1)

        elif clickfunc == 'toggle-multiple-cells':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt

            self.toggle_cell(clickarg,expose=1)

        elif clickfunc == 'start-cell-edit':
            if clickstr not in _cell_clickevents:
                txt='pgugrid: cell cannot be selected in response\n'
                txt=txt+'          to a row or column click event.'
                raise RuntimeError,txt
            self.start_cell_edit(clickarg)

        elif clickfunc == 'sort-by-column':
            # Sort by column (ascending)
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.sort_reverse=0
            if clickstr in _column_clickevents:
                self.sort_by_column(clickarg[0],expose=1)
            else: 
                self.sort_by_column(clickarg[1],expose=1)

        elif clickfunc == 'reverse-sort-by-column':
            # Sort by column (descending)
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.sort_reverse=1
            if clickstr in _column_clickevents:
                self.sort_by_column(clickarg[0],expose=1)
            else: 
                self.sort_by_column(clickarg[1],expose=1)       
        elif clickfunc == 'toggle-sort-by-column':
            # Sort by column (alternate between ascending
            # and descending with consecutive calls).  If
            # a new column is being sorted, start with
            # ascending sort regardless of the ascending/descending
            # state for the last column sorted.
            if clickstr in _row_clickevents:
                txt='pgugrid: column cannot be selected in response\n'
                txt=txt+'         to a row click event'
                raise RuntimeError,txt

            self.sort_reverse=abs(self.sort_reverse-1)
            if clickstr in _column_clickevents:
                if self.sort_column != clickarg[0]:
                    self.sort_reverse=0
                self.sort_by_column(clickarg[0],expose=1)
            else:
                if self.sort_column != clickarg[1]:
                    self.sort_reverse=0 
                self.sort_by_column(clickarg[1],expose=1)
        elif clickfunc == 'translate-view-to-row':
            # If source type is shapes layer, translate view
            # to that row.
            if clickstr in _column_clickevents:
                txt='pgugrid: row cannot be selected in response\n'
                txt=txt+'         to a column click event'
                raise RuntimeError,txt

            self.translate_view_to_row(clickarg[0])

        else:
            raise '_perform_click_action: unrecognized click function'


    def changed( self, widget ):
        """Track changes to the scrollbars and record the 
        """
        self.start_row = int(self.vadj.value)
        self.start_column = int(self.hadj.value)
        self.expose()

    def refresh( self, *args ):
        """ Refresh grid from source (use if source has been
            internally or externally changed).
        """
        # cancel any editing operations
        if self.editing_cell is not None:
            self.cancel_cell_edit()
            self.unselect_all_cells(expose=0)

        if self.src_type == SRC_LISTUNDEF:
            if len(self.src) == 0:
                self.expose()
                return

            if type(self.src[0]) == type([]):
                self.src_type=SRC_LISTLIST
            elif type(self.src[0]) == type((1,)):
                self.src_type=SRC_LISTLIST
            else:
                self.src_type=SRC_LISTOBJ

        self._generate_row_indices()

        self.expose()


    def resize_to_default(self,max_width=400,max_height=300):
        """ Resize to the default size calculated by
            get_default_size, tp a maximum width of max_width
            and maximum height of max_height.
        """

        newsize=self.get_default_size(max_width,max_height)
        self.set_size_request(*newsize)


    def select_next_row(self,clearfirst=1,expose=1):
        """If a row is selected, select the next
           row in the current sorting scheme.  If no row
           is currently selected, do nothing.  Uses the
           last selected row if multiple rows are selected.
           Clears all other selections if clearfirst is 1.
        """
        if len(self.selected_rows) == 0:
            return

        srow=self.selected_rows[len(self.selected_rows)-1]

        if srow not in self.row2src:
            # selected row is not in the display.  do nothing.
            return

        g_row=self.src2row[srow]
        g_row=g_row+1
        if g_row >= len(self.row2src):
            g_row=0

        nsrow=self.row2src[g_row]
        self.select_row(nsrow,clearfirst,expose)

    def grid2src(self,grid_row):
        """ Convert grid row to source row (returns -1 if the
            row is out of range)
        """

        if type(grid_row) in [type([]),type(())]:
            if len(grid_row) == 0:
                return ()

            garr=Numeric.array(grid_row)
            mask=Numeric.where(garr >= len(self.row2src),
                               -1,
                               Numeric.where(garr<0,-1,1))
            garr=Numeric.where(garr >= len(self.row2src),
                               0,
                               Numeric.where(garr<0,0,garr))
            sarr=Numeric.where(mask>0,Numeric.take(self.row2src,list(garr)),
                               mask)
            return tuple(sarr)
        else:
            if (grid_row > -1) and (grid_row < len(self.row2src)):
                return self.row2src[grid_row]
            else:
                return -1

    def src2grid(self,src_row):
        """ Convert source row to grid row (returns -1 if source
            row is not in grid- this may be the case if a subset
            is specified or if the source row is None or if the
            row is out of range)
        """

        if type(src_row) in [type([]),type(())]:
            if len(src_row) == 0:
                return ()

            sarr=Numeric.array(src_row)
            mask=Numeric.where(sarr >= len(self.src2row),
                               -1,
                               Numeric.where(sarr<0,-1,1))
            sarr=Numeric.where(sarr >= len(self.src2row),
                               0,
                               Numeric.where(sarr<0,0,sarr))
            garr=Numeric.where(mask>0,Numeric.take(self.src2row,list(sarr)),
                               mask)
            garr=Numeric.maximum(garr,-1)

            return tuple(garr)
        else:
            if (src_row > -1) and (src_row < len(self.src2row)):
                return max(self.src2row[src_row],-1)
            else:
                return -1

    def get_current_columns(self):
        """ Return the current column info.  Is a tuple
            of lists:
            (members,titles,editables,formats,types,
            nodatas,justifys,title_justifys,widths,start_x's,force_widths,
            force_width_chars, entry_chars)
        """
        members=[]
        titles=[]
        editables=[]
        formats=[]
        types=[]
        nodatas=[]
        justifys=[]
        title_justifys=[]
        widths=[]
        start_xs=[]
        force_widths=[]
        force_width_chars=[]
        entry_chars=[]
        for item in self._ColumnDefs:
            members.append(item.member)
            titles.append(item.title)
            editables.append(item.editable)
            formats.append(item.format)
            types.append(item.type)
            nodatas.append(item.nodata)
            justifys.append(item.justification)
            title_justifys.append(item.title_justification)
            widths.append(item.width)
            start_xs.append(item.start_x)
            force_widths.append(item.force_width)
            force_width_chars.append(item.force_width_chars)
            entry_chars.append(item.entry_chars)

        return (members,titles,editables,formats,types,nodatas,
                justifys,title_justifys,widths,start_xs,force_widths,
                force_width_chars, entry_chars)


    def get_default_size(self,max_width=400,max_height=300):
        """ Calculate a sensible size to allocate for the grid.
            If size is greater than max_width x max_height,
            try to break on column and row boundaries.
            Returns a (width,height) tuple.
        """

        if len(self._ColumnDefs) == 0:
            return (max_width,max_height)

        # offsets account for scrollbars and finite line width
        if self.vscroll_shown == 1:
            cwidth=self._ColumnDefs[self.g_columns-1].start_x+\
                   self._ColumnDefs[self.g_columns-1].width+20
        else:
            cwidth=self._ColumnDefs[self.g_columns-1].start_x+\
                   self._ColumnDefs[self.g_columns-1].width+2

        idx=self.g_columns-1
        while ((cwidth > max_width) and (idx > 0)):
            cwidth=cwidth-self._ColumnDefs[idx].width-1
            idx=idx-1

        cwidth=min([cwidth,max_width])

        if self.hscroll_shown == 1: 
            cheight=(self.row_height+1)*len(self.row2src)+20
        else:
            cheight=(self.row_height+1)*len(self.row2src)+2

        if self.opts.show_column_titles == 1:
            cheight=cheight+self.column_title_height+1

        if (cheight > max_height):
            cheight=cheight-((self.row_height+1)*(int(
             Numeric.ceil(float((cheight-max_height))/(self.row_height+1)))))

        return (cwidth,cheight)

    def expose( self, *args ):
        """Draw the widget
        """
        if not (self.flags() & gtk.REALIZED):
            return

        if self._flags['frozen'] == 1:
            return

        #
        # create a memory pixmap to render into
        #
        try:
            win=self._area.window
            width, height = win.get_size()
        except:
            print traceback.print_exc()
            return

        pix = self._pixmap

        #
        # prefetch the style
        #
        style = self.get_style()

        if self.style_list[0] is None:
            # Start by creating default styles.  Note that before,
            # gtk.STATE_PRELIGHT was used to indicate selected cells,
            # and gtk.STATE_SELECTED was used for titles (regardless
            # of selection).  By default, revert back to this
            # convention.
            cellstyle=style.copy()
            cellstyle.bg_gc[gtk.STATE_NORMAL]=style.white_gc

            cellstyle.bg_gc[gtk.STATE_SELECTED]=style.bg_gc[gtk.STATE_PRELIGHT]
            self.style_list[0]=cellstyle

            # row titles
            titlestyle=style.copy()
            titlestyle.bg_gc[gtk.STATE_NORMAL]=style.bg_gc[gtk.STATE_NORMAL]
            self.style_list[1]=titlestyle

            # column titles
            titlestyle=style.copy()
            titlestyle.bg_gc[gtk.STATE_NORMAL]=style.bg_gc[gtk.STATE_NORMAL]
            self.style_list[2]=titlestyle

        if self.default_style_reset_flag == 1:
            # User has reset default style
            self._add_style(0,self.default_style)
            self.default_style_reset_flag = 0

        if self.default_row_title_style_reset_flag == 1:
            # User has reset default row title style
            self._add_style(1,self.default_row_title_style)
            self.default_row_title_style_reset_flag = 0

        if self.default_col_title_style_reset_flag == 1:
            # User has reset default column title style
            self._add_style(2,self.default_col_title_style)
            self.default_col_title_style_reset_flag = 0

        if self.styles_to_add is not None:
            for item in self.styles_to_add:
                self._add_style(item[0],item[1])

            self.styles_to_add=None

        # Note on drawing order:
        #
        # - background (cell style default background)
        # - empty message (if relevant): returns
        # - If row titles present, draw the rectangle
        #   background covering all row titles, then
        #   text row number of first row title, second
        #   row title, etc.
        # - For i in range (first column, last column+1)
        #       - If column titles are present, draw the
        #         rectangle for the current column title
        #         background, then the text.
        #       - For each cell in the column, draw the background
        #         rectangle, then the text.
        #

        #
        # clear the pixmap
        #
        #print "8: draw_rectangle"
        #gtk.draw_rectangle( pix, self.style_list[0].bg_gc[gtk.STATE_NORMAL],
        #                    True, 0, 0, width, height )
        pix.draw_rectangle( self.style_list[0].bg_gc[gtk.STATE_NORMAL],
                            True, 0, 0, width, height )

        #print 'exposing...'
        #print 'pix: ',self._pixmap
        if ((self.src_type in [SRC_NONE, SRC_LISTUNDEF]) and
            (len(self._ColumnDefs) == 0)):
            msg = self.empty_msg
            if (msg is not None) and (len(msg) > 0):
                msg_width = self.title_font.string_width( msg )
                msg_height = self.title_font.height( msg )
                msg_x = (width / 2) - (msg_width / 2)
                msg_y = (height / 2) + (msg_height / 2 )
                #print '5: draw_text'
                pix.draw_text( self.title_font, 
                           style.fg_gc[gtk.STATE_INSENSITIVE], 
                           msg_x, msg_y, msg )
            win.draw_drawable(style.white_gc, self._pixmap, 
                                   0, 0, 0, 0, 
                                   width, height )

            if self.bCalcAdjustments:
                self.calc_adjustments()

            return False


        #
        # track changes in column width because of wide columns
        #
        bResetAdj = False

        #
        # calculate the number of rows to draw
        #
        base_height = height
        column_title_height = self.column_title_height
        data_height = base_height - column_title_height - 1
        if self.opts.show_column_titles == 1:
            data_height=data_height-1

        disp_rows = int(Numeric.floor(
            data_height / ( self.row_height + 1 ))) + 1

        first_row = self.start_row
        last_row = first_row + disp_rows - 1
        if last_row > self.g_rows-1:
            last_row = self.g_rows-1
            disp_rows=last_row-first_row+1

        first_column = self.start_column
        last_column = self.g_columns

        sum_row_heights=disp_rows*(self.row_height+1)

        if self.opts.show_row_titles == 1:
            rt_height= disp_rows*( self.row_height + 1 ) + \
                       self.column_title_height
            rt_offset=self.column_title_height + self.row_height + 1 - \
                      self.pad

            if self.opts.show_column_titles == 1:
                rt_height = rt_height + 1
                rt_offset = rt_offset + 1

            #print "1: draw_rectangle"
            pix.draw_rectangle(self.style_list[1].bg_gc[gtk.STATE_NORMAL],
                               True,0,0,
                               self.row_title_width+1,
                               rt_height+1)

            for c_row in range(first_row,last_row+1):
                if self.opts.row_title_type == 'source':
                    #print '1: draw_text'
                    pix.draw_text(self.title_font,
                                  style.fg_gc[gtk.STATE_NORMAL],
                                  self.pad,rt_offset,str(self.row2src[c_row]))
                else:
                    #print '2: draw_text'
                    pix.draw_text(self.title_font,
                                  style.fg_gc[gtk.STATE_NORMAL],
                                  self.pad,rt_offset,str(c_row))

                rt_offset = rt_offset + self.row_height + 1


        #
        # starting x for the the first column (far left line)
        #
        x = self.row_title_width
        if self.opts.show_row_titles == 1:
            x = x+1


        #
        # loop through a column at a time
        #
        row_is_selected=[]

        rstyles=[]   # Draw styles
        cstyles=[]
        cjust=[]   # Column justification
        ctjust=[]   # Column title justification
        for i in range( first_column, last_column ):
            #
            # don't bother drawing if we're going to start past the right edge
            #
            if x > width:
                continue

            cells=[]            
            # Info on whether or not shapes are selected (1 if selected)
            cell_is_selected = []
            cstyles.append(self.style_list[self.column_style_index[i]])
            cjust.append(self._ColumnDefs[i].justification)
            ctjust.append(self._ColumnDefs[i].title_justification)
            force_width=self._ColumnDefs[i].force_width

            for j in range( first_row, last_row+1 ):
                idx = self.row2src[j]

                if idx == -1:
                    continue

		txt=self._get_datastr(idx,i)

                if txt is None:
                    txt = ""

                if force_width is not None:
                    cell_width = force_width
                    real_cell_width = min(force_width,
                                 self.cell_font.string_width( txt ) + 2*self.pad)
                else:
                    cell_width = self.cell_font.string_width( txt ) + 2*self.pad
                    real_cell_width = cell_width

                cells.append( (txt, real_cell_width) )
                cell_is_selected.append(
                    self.cell_selectstate[idx,i])

                if i == first_column:
                    row_is_selected.append(self.row_selectstate[idx])
                    rstyles.append(self.style_list[self.row_style_index[idx]])

                if cell_width > self.column_widths[i]:
                    bResetAdj = True
                    self.column_widths[i] = cell_width
                    self._update_column_width(i,cell_width)

            #
            # figure out the size and placement of the title text
            #
            y = 1
            if force_width is not None:
                title_width=force_width
                real_title_width = min(force_width,
                        self.title_font.string_width(self._ColumnDefs[i].title ) +
                                       2*self.pad)
            else:
                title_width = self.title_font.string_width(
                        self._ColumnDefs[i].title ) + 2*self.pad
                real_title_width = title_width

            if title_width > self.column_widths[i]:
                bResetAdj=True
                self.column_widths[i]=title_width
                self._update_column_width(i,title_width)

            #
            # draw the 'button'
            #
            bx = x
            by = 0 
            bw = self.column_widths[i]+1
            if self.opts.show_column_titles == 1:
                bh = self.column_title_height+1
            else:
                bh = 0

            if self.opts.show_column_titles == 1:
                #print "2: draw_rectangle"
                pix.draw_rectangle( self.style_list[2].bg_gc[gtk.STATE_NORMAL],
                                    True, bx, by, bw, bh)

                #
                # draw the title
                #
                if ctjust[i-first_column] == 0:
                    tx = x + ( self.column_widths[i] - real_title_width )
                elif ctjust[i-first_column] == 1:
                    tx = x
                else:
                    tx = x + ( ( self.column_widths[i] -
                                 real_title_width ) / 2 )

                #print '3: draw_text'
                pix.draw_text( self.title_font, 
                               style.fg_gc[gtk.STATE_NORMAL], 
                               tx+self.pad,
                               y + self.column_title_height - self.pad,
                               self._ColumnDefs[i].title )

                y = y+self.column_title_height + 1

            #
            # draw the horizontal line below the title
            #


            for j in range( len(cells) ): 
                if cell_is_selected[j]:  
                    #print "3: draw_rectangle"
                   pix.draw_rectangle( rstyles[j].bg_gc[gtk.STATE_SELECTED],
                                       True, 
                                       x, 
                                       y, 
                                       self.column_widths[i]+1,
                                       self.row_height+1)                    
                elif self.column_selectstate[i]:
                    #print "4: draw_rectangle"
                   pix.draw_rectangle( cstyles[i-first_column].bg_gc[gtk.STATE_SELECTED],
                                       True, 
                                       x, 
                                       y, 
                                       self.column_widths[i]+1,
                                       self.row_height+1)                     
                elif row_is_selected[j]:
                    #print "5: draw_rectangle"
                   pix.draw_rectangle( rstyles[j].bg_gc[gtk.STATE_SELECTED],
                                       True, 
                                       x, 
                                       y, 
                                       self.column_widths[i]+1,
                                       self.row_height+1)  
                else:
                    #print "6: draw_rectangle"
                   pix.draw_rectangle( rstyles[j].bg_gc[gtk.STATE_NORMAL],
                                       True, 
                                       x, 
                                       y, 
                                       self.column_widths[i]+1,
                                       self.row_height+1)  

                y = y + self.row_height + 1

                if cjust[i-first_column] == 0:
                    cx = x + self.column_widths[i] - cells[j][1]
                elif cjust[i-first_column] == 1:
                    cx = x
                else:
                    cx = x + ( (self.column_widths[i] - cells[j][1]) / 2 )

                #print '4: draw_text'
                pix.draw_text(self.cell_font, 
                              style.fg_gc[gtk.STATE_NORMAL], 
                              cx+self.pad, y-self.pad, cells[j][0])


            #
            # where does the line go
            #
            ly = y - 1
            lx = x + self.column_widths[i] + 1

            #
            #advance to next column
            #
            x = x + self.column_widths[i] + 1

        # Redraw the area to the right of the last column in case there
        # were any text over-runs:
        #print "7: draw_rectangle"
        pix.draw_rectangle( self.style_list[0].bg_gc[gtk.STATE_NORMAL],
                            True, 
                            x+1, 
                            0, 
                            width-x-1,
                            height)          


        # Different line drawing options- represent with one
        # number for shorter if's.
        # draw all the lines (none for dlcase == 0)
        dlcase = self.draw_row_lines*3 + self.draw_col_lines
        if (dlcase == 1) and (self.opts.show_column_titles == 0):
            dlcase = 0
        elif (dlcase == 3) and (self.opts.show_row_titles == 0):
            dlcase = 0
        elif ((dlcase == 4) and (self.opts.show_row_titles == 0) and
              (self.opts.show_column_titles == 0)):
            dlcase = 0
        elif ((dlcase == 4) and (self.opts.show_row_titles == 1) and
              (self.opts.show_column_titles == 0)):
            dlcase = 3
        elif ((dlcase == 4) and (self.opts.show_row_titles == 0) and
              (self.opts.show_column_titles == 1)):
            dlcase = 1
        elif ((dlcase == 5) and (self.opts.show_row_titles == 0) and
              (self.opts.show_column_titles == 0)):
            dlcase = 2

        # calculate total width/height of grid
        if self.opts.show_row_titles == 0:
            gwidth = 0
        else:
            gwidth = self.row_title_width + 1

        for i in range( first_column, last_column):
            gwidth = gwidth + self.column_widths[i] + 1

        if self.opts.show_column_titles == 0:
            gheight = 0
        else:
            gheight = self.column_title_height + 1

        for i in range(first_row, last_row+1):
            gheight = gheight + self.row_height + 1

        if dlcase in [2,4,5,6,7,8]:
            # lines left, right, top, bottom
            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          0,0,gwidth,0)
            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          0,gheight,gwidth,gheight)
            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          0,0,0,gheight)
            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          gwidth,0,gwidth,gheight)

        if dlcase in [1,4,5,7,8] and (self.opts.show_column_titles == 1):
            # Lines around column titles
            if self.opts.show_row_titles == 0:
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,0,gwidth,0)
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,self.column_title_height+1,
                              gwidth,self.column_title_height+1)
                roffset = 0
            else:
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              self.row_title_width+1,0,gwidth,0)
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              self.row_title_width+1,
                              self.column_title_height+1,
                              gwidth,self.column_title_height+1)
                roffset = self.row_title_width + 1

            for i in range( first_column, last_column):
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              roffset,0,
                              roffset,self.column_title_height+1)
                roffset = roffset + self.column_widths[i] + 1

            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          roffset,0,
                          roffset,self.column_title_height+1)

        if (dlcase in [3,4,5,7,8]) and (self.opts.show_row_titles == 1):
            # Lines around row titles
            if self.opts.show_column_titles == 0:
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,0,0,gheight)
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              self.row_title_width+1,0,
                              self.row_title_width+1,gheight)
                coffset = 0
            else:
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,self.column_title_height+1,0,gheight)
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              self.row_title_width+1,
                              self.column_title_height+1,
                              self.row_title_width+1,gheight)
                coffset = self.column_title_height + 1

            for i in range( first_row, last_row+1):
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,coffset,
                              self.row_title_width+1,coffset)
                coffset = coffset + self.row_height + 1

            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          0,coffset,
                          self.row_title_width+1,coffset)

        if dlcase in [2,5,8]:
            # lines between columns
            if self.opts.show_row_titles == 0:
                roffset = 0
            else:
                roffset = self.row_title_width + 1

            for i in range( first_column, last_column):
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              roffset,0,
                              roffset,gheight)
                roffset = roffset + self.column_widths[i] + 1

            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          roffset,0,
                          roffset,gheight)

        if dlcase in [6,7,8]:
            # lines between rows
            if self.opts.show_column_titles == 0:
                coffset = 0
            else:
                coffset = self.column_title_height + 1

            for i in range( first_row, last_row+1):
                pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                              0,coffset,
                              gwidth,coffset)
                coffset = coffset + self.row_height + 1
            pix.draw_line(style.fg_gc[gtk.STATE_INSENSITIVE],
                          0,coffset,
                          gwidth,coffset)


        #draw the backing pixmap onto the screen
        win.draw_drawable(style.white_gc, self._pixmap, 0, 0, 0, 0, 
                               width, height )

        if self.editing_cell is not None:
            # shift editing cell if necessary
            cell=self.editing_cell
            if self.src2row[cell[0]] < first_row:
                self._layout.move(self.editbox,5000,5000)
            elif cell[1] < first_column:
                self._layout.move(self.editbox,5000,5000)
            else:
                cwidth=self._ColumnDefs[cell[1]].width
                self.editbox.set_size_request(cwidth,self.row_height)
                self.editbox.set_max_length(
                    self._ColumnDefs[cell[1]].entry_chars)
                locx=self._ColumnDefs[cell[1]].start_x-\
                      self._ColumnDefs[first_column].start_x+\
                      self.row_title_width+1
                if self.opts.show_row_titles == 1:
                    locx=locx+1
                locy=self.column_title_height+2+\
                  ((self.src2row[cell[0]]-self.start_row)*(self.row_height+1))
                if self.opts.show_column_titles == 1:
                    locy=locy+1

                self._layout.move(self.editbox,locx,locy)

        if bResetAdj or self.bCalcAdjustments:
            self.calc_adjustments()


        return False

    def size_allocate(self,*args):
        """Track changes in table size and pass them on to
           drawing area.
        """

        nsizetuple=self.get_allocation()
        if self.vscroll_shown == 1:
            nwidth=nsizetuple[2]-18
        else:
            nwidth=nsizetuple[2]

        if self.hscroll_shown == 1:
            nheight=nsizetuple[3]-18
        else:    
            nheight=nsizetuple[3]

        self._area.set_size_request(nwidth,nheight)

    def configure( self, widget, event, *args ):
        """Track changes in width, height
        """
        #only do this if we have been realized
        if not self.flags() & gtk.REALIZED:
            return

        # create a memory pixmap to render into
        ##a_win = self._area.window
        a_win = self._area.window
        w,h = a_win.get_size()
        self._pixmap = gtk.gdk.Pixmap( a_win, w, h)

        style = self.get_style()

        if self.title_font is None:
            try:
                self.title_font = gtk.load_font( self.title_font_spec )
            except:
                ##traceback.print_exc()
                self.title_font = style.get_font()
            if self.opts.show_column_titles == 1:     
                self.column_title_height = self.title_font.ascent + 2*self.pad
            else:
                self.column_title_height = 0


            if self.opts.show_row_titles == 1:
                if (self.src is not None):
                    self.row_title_width=self.title_font.string_width(
                        str(len(self.src2row)))+2*self.pad
                else:
                    self.row_title_width=0
            else:
                self.row_title_width=0

        if self.cell_font is None:
            try:
                self.cell_font = gtk.load_font( self.cell_font_spec )
            except:
                ##print traceback.print_exc()
                self.cell_font = style.get_font()
            self.row_height = self.cell_font.ascent + 2*self.pad

        self.bCalcAdjustments=True

    def calc_adjustments( self ):
        """Recalculate the adjustment settings
        """
        if not (self.flags() & gtk.REALIZED):
            self.bCalcAdjustments = True
            return

        self.updating = True
        #horizontal min/max are 0 and max line length - page size
        hpos = self.hadj.value

        h_min = 0

        # Extra 1's account for 1-pixel wide lines between cells;
        w,h = self._area.window.get_size()
        win_width = w - 1 - self.row_title_width

        if self.opts.show_row_titles == 1:
            win_width = win_width - 1

        # Number of columns in window will change if columns
        # have different widths, so calculate how many
        # clicks it will take to show all of the last column
        # assuming win_width > width(last_column).
        # rather than just setting hadj to the number of
        # columns not visible initially.  Note: one click
        # should move grid over by one column.
        needed_clicks=0

        if len(self.column_widths) > 0:
            while ((Numeric.sum(Numeric.array(
                self.column_widths[needed_clicks:])+1) > win_width) and
                (needed_clicks < self.g_columns-1)):
                needed_clicks=needed_clicks+1


        h_max=needed_clicks+1

        self.hadj.set_all( hpos, h_min, h_max, 1, 1, 1 )
        self.hadj.changed()

        vpos = self.vadj.value
        v_min = 0
        cells_height = h - \
                     self.column_title_height - 1

        if self.opts.show_column_titles == 1:
            cells_height=cells_height - 1

        rows = Numeric.floor(float(cells_height) / (self.row_height+1))

        v_max=self.g_rows

        v_max = max( 0, v_max - rows ) + 1

        self.vadj.set_all( vpos, v_min, v_max, 1, 1, 1)
        self.vadj.changed()

        if (self.hscroll_policy == 0) and (self.hscroll_shown != 1):
            self.hsframe.show()
            self.hscroll_shown = 1
        elif (self.hscroll_policy == 2) and (self.hscroll_shown != 0):
            self.hsframe.hide()
            self.hscroll_shown = 0
        elif (self.hscroll_policy == 1):    
            if (needed_clicks == 0) and (self.hscroll_shown != 0):
                self.hsframe.hide()
                self.hscroll_shown = 0
            elif (needed_clicks != 0) and (self.hscroll_shown != 1):
                self.hsframe.show()
                self.hscroll_shown = 1

        if (self.vscroll_policy == 0) and (self.vscroll_shown != 1):
            self.vsframe.show()
            self.vscroll_shown = 1
        elif (self.vscroll_policy == 2) and (self.vscroll_shown != 0):
            self.vsframe.hide()
            self.vscroll_shown = 0
        elif (self.vscroll_policy == 1):    
            if (v_max - v_min == 1) and (self.vscroll_shown != 0):
                self.vsframe.hide()
                self.vscroll_shown = 0
            elif (v_max - v_min > 1) and (self.vscroll_shown != 1):
                self.vsframe.show()
                self.vscroll_shown = 1 

        self.bCalcAdjustments = False
        self.updating = False

    def set_scroll_policy(self, hpolicy, vpolicy, expose=1):
        """ Set the policy for showing horizontal
            and vertical scrollbars.

            Inputs:
                hpolicy- integer
                vpolicy- integer
                expose- whether or not to immediately
                        redraw the grid (0 to not redraw,
                        1 to redraw- defaults to 1)

            Policy values: 0- always
                           1- automatic
                           2- never

            Policy is 0 (always) by default.
        """
        self.hscroll_policy = hpolicy
        self.vscroll_policy = vpolicy
        self.bCalcAdjustments = True
        if expose == 1:
            self.expose()

    def clear(self,*args):
        """ Clear all grid settings, disconnect from
            source signals.
        """

        # End cell editing, if it is occurring (save changes)
        self.end_cell_edit()

        # Disconnect from layer signals (SRC_SHAPESLAYER only)
        if self.layer_selection_changed_id is not None:
            self.layer.disconnect(self.layer_selection_changed_id)
            self.layer_selection_changed_id = None
        if self.layer_subselection_changed_id is not None:
            self.layer.disconnect(self.layer_subselection_changed_id)
            self.layer_subselection_changed_id = None
        if self.layer_teardown_id is not None:
            self.layer.disconnect(self.layer_teardown_id)
            self.layer_teardown_id = None

        # Disconnect from changed signals (SRC_SHAPES, SRC_SHAPESLAYER)    
        if ((self.source_changed_id is not None) and
           (self.src is not None)):
            self.src.disconnect(self.source_changed_id)
            self.source_changed_id=None
        elif (self.source_changed_id is not None):
            print 'clear: Warning- encountered undisconnected signal'

        # Clear settings
        self._initialize_settings()

    def clear_and_expose(self,*args):
        """ Clear and expose. """
        self.clear()
        self.expose()

    def _initialize_settings(self):
        """ Set up the grid for a new source. """

        #the row/column to put in the top left corner
        self.start_row = 0
        self.start_column = 0

        self.last_row = 0

        self.column_widths = []

        #flag to recalculate the adjustments
        self.bCalcAdjustments = True

        # Lists of currently selected rows,
        # columns, cells.  Selection state
        # is stored two ways for easy
        # access in different situations.
        # Selected rows always refer to
        # the source row, not the grid row.
        # (this makes it easier to deal
        # with sorting)
        # Selected column refers to the
        # grid column because the underlying
        # source may have no concept of
        # "column" (eg. shapes have properties,
        # but there is no intrinsic order to
        # them).
        # Cells are referenced by the source
        # row and grid column (a tuple)
        self.selected_rows=[] 
        self.selected_columns=[]
        self.selected_cells=[]

        # Select state indices.  Each element
        # of these is either:
        #     0- not selected
        #         or
        #     1- selected.
        # For rows and columns these are vectors,
        # for the cells this is a matrix.
        # The sizes are:
        # row_selectstate: number of source rows (nR)
        # column_selectstate: number of grid columns (nC)
        # cell_selectstate: nR x nC
        #
        # row_selectstate refers to the source row
        # in the original source order (ie. unsorted)
        # column_selectstate refers to the column index
        # in the grid display.
        # cells are indexed by source row (unsorted) and
        # displayed column index.
        self.row_selectstate=None
        self.column_selectstate=None
        self.cell_selectstate=None

        # Selection tracking
        self.last_selected_row=None
        self.last_selected_column=None
        self.last_selected_cell=None
        self.last_toggled_row=None
        self.last_toggled_column=None
        self.last_toggled_cell=None

        # click event tracking (hack used to get
        # around the fact that double clicking
        # doesn't seem to work well on windows for longish callbacks-
        # disadvantage is that the consecutive clicks
        # can be arbitrarily far apart...).  Hack is used on all
        # platforms for consistency.
        self.last_click_cell=(None,None)
        self.last_click_row=None
        self.last_click_column=None
        self.last_click_button=None

        #set to true if changing some value that would end up causing multiple
        #expose events or an endless loop even.
        self.updating = False
        # Define source parameters; initialize them
        # to empty values
        self.src=None
        self.src_type=SRC_NONE

        # Indices to map from source index to
        # grid row index, and vice versa.
        # -2 is in src2row to indicate that
        # the source data for that row is
        # None and should not be used.  -1
        # is used to indicate that the
        # source data for that row is not
        # part of the current subset and
        # should not be used.
        self.src2row=[]
        self.row2src=[]

        # More parameters
        self.g_columns=0 # Number of grid columns
        self.g_rows=0 # Number of grid rows
        self.s_rows=0 # Number of source rows

        # Sorting parameters
        self.sort_column=-1  # No sorting to start with
        self.sort_reverse=0  # ascending (0) or descending (1)

        # source changed: used for SRC_SHAPES
        # and SRC_SHAPESLAYER
        self.source_changed_id=None

        # layer and view: only used for
        # SRC_SHAPESLAYER
        self.layer=None
        self.layer_selection_changed_id=None
        self.layer_subselection_changed_id=None
        self.layer_teardown_id=None
        self.view=None

        # Flags used to control callback behaviour in
        # different contexts (eg. to avoid exposes
        # in row selection if necessary)
        self._flags={}
        self._flags['selecting-rows']=0
        self._flags['frozen']=0

        self.row_style_index=None           
        self.column_style_index=None        
        # Below: not used yet, but will be later.
        #self.rowtitle_style_index=None
        #self.columntitle_style_index=None

        self.editing_cell=None

    def layer_subselection_cb(self,layer,*ignored):
        # When a selection changes, a the selected list is changed, 
        # then the selection-changed signal is sent out, then
        # the subselection changes, then the subselection-changed
        # signal is sent out. To reset the start row, you have to hook
        # into the subselection-changed callback, since all
        # selection-changed callbacks are executed before
        # subselection is updated, so calling get_subselection from the
        # selection-changed callback will lag the most recent selection
        # by one.
        subselection = layer.get_subselected()

        if len(self.src) < self.s_rows:
            # selection changed signal is sent
            # out before shapes changed signal
            # when an invalid area is drawn.
            # Return if this is the case.
            return

        if subselection != -1:
            self.last_selected_row=subselection
            self.last_toggled_row=subselection
            self.reset_startrow(subselection)

        # If this selection was triggered by actions
        # on the view rather than through the grid,
        # redraw the grid (if this was triggered through
        # the grid this function is called from select_rows,
        # which does the expose if necessary).
        if self._flags['selecting-rows'] == 0:
            self.expose()

    def layer_selection_cb(self, layer, *args):
        """ Shapeslayer case- selection has been changed in view. """
        shps = layer.get_selected()

        if len(self.src) < self.s_rows:
            # selection changed signal is sent
            # out before shapes changed signal
            # when an invalid area is drawn.
            # Return if this is the case.
            return

        if len(shps) == 1:
            self.last_selected_row=shps[0]
            self.last_toggled_row=shps[0]
        else:
            self.last_selected_row=None
            self.last_toggled_row=None

        self._unselect_all_rows()    
        self._select_rows(shps)

        if self._flags['selecting-rows'] == 0:

            nlist=self._rows_updated()

            if (len(shps) == 0) or (len(nlist) > 0):
                # If no shapes are selected, layer_subselection_cb
                # won't be called, so layer_selection_cb should do
                # the expose (otherwise layer_subselection_cb will)
                self.expose()

            Signaler.notify(self, 'row-selection-changed',tuple(self.selected_rows))

            if len(nlist) > 0:
                for item in nlist:
                    Signaler.notify(self, item[0],item[1])

    def freeze(self):
        """ Freeze and allow internal changes without exposing. """
        if self.source_changed_id is not None:
            self.src.handler_block(self.source_changed_id)
        if self.layer_selection_changed_id is not None:
            self.src.handler_block(self.layer_selection_changed_id)
        if self.layer_subselection_changed_id is not None:
            self.src.handler_block(self.layer_subselection_changed_id)
        self._flags['frozen']=1

    def thaw(self,expose=1):
        """ Thaw and expose if desired. """
        if self.source_changed_id is not None:
            self.src.handler_unblock(self.source_changed_id)
        if self.layer_selection_changed_id is not None:
            self.src.handler_unblock(self.layer_selection_changed_id)
        if self.layer_subselection_changed_id is not None:
            self.src.handler_unblock(
                self.layer_subselection_changed_id)
        self._flags['frozen']=0

        if expose == 1:
            self.expose()

    def sort_by_column( self, column=None,reverse=None,expose=1 ):
        """ Sort the grid rows according to the
            values in one of the columns.
        """

        if ((self.src is None) or (len(self.src) < 1)):
            return

        if column is None:
            # default to last one if sort property not specified
            column=self.sort_column
        else:
            self.sort_column=column

        if reverse is None:
            reverse=self.sort_reverse
        else:
            self.sort_reverse=reverse

        if column > len(self._ColumnDefs):
            txt='pgugrid: attempted to sort by nonexistent column '+str(column)
            raise RuntimeError,txt

        ind_list=[]
        if self._ColumnDefs[self.sort_column].type == 'complex':
            # complex numbers can't be sorted with list sorting
            for s_row in self.row2src:
                ind_list.append((self._get_datastr(s_row,self.sort_column),
                                s_row))
        else:
            for s_row in self.row2src:
                ind_list.append((self._get_data(s_row,self.sort_column),s_row))

        ind_list.sort()
        if self.sort_reverse == 1:
            ind_list.reverse()

        for idx in range(len(ind_list)):
            self.row2src[idx]=ind_list[idx][1]
            self.src2row[ind_list[idx][1]]=idx


        if expose == 1:
            if self.last_selected_row is not None:
                self.reset_startrow(self.last_selected_row)

            self.expose()


class pguGridWin(gtk.Window):
    def __init__(self,title,selection_mode=1,source=None,config=None):
        """ selection mode: 0 for no row selection, 1
            for single selection, 2 for multiple.

            source: source to initialize grid with
        """

        gtk.Window.__init__(self)
        self.set_title(title)
        if config is None:
            if selection_mode == 0:
                config=(2,0,0,0,0,0,0,0)
            elif selection_mode == 1:
                config=(2,1,0,0,0,0,0,0)
            else:
                config=(2,2,0,0,0,0,0,0)

        self.grid=pguGrid(config)
        self.add(self.grid)

        self.set_resizable(True)
        if source is not None:
            self.set_source(source)

        self.show_all()
        if source is not None:
            self.grid.resize_to_default(800,600)


    def set_source(self,src,view=None):
        self.grid.set_source(src,view)

    def set_subset(self,indices):
        self.grid.set_subset(indices)


class pguTestGridWin(gtk.Window):
    def __init__(self,title,config=None,source=None):
        gtk.Window.__init__(self)
        self.set_title(title)
        if config is None:
            config=(2,1,0,1,0,1,0,0)
        self.grid=pguGrid(config)
        self.add(self.grid)

        self.set_resizable(True)
        if source is not None:
            self.set_source(source)
            self.grid.resize_to_default()
            self.show_all()


    def set_source(self,src,view=None):
        self.grid.set_source(src,view)
        self.grid.resize_to_default()
        self.show_all()

    def set_subset(self,indices):
        self.grid.set_subset(indices)


class _test_listobj:
    def __init__(self,strmem,intmem,floatmem,cplxmem):
        self.stringval=strmem
        self.intval=intmem
        self.floatval=floatmem
        self.complexval=cplxmem
        self.array=Numeric.array([1,2,3])

    def dummy_function(self):
        pass

def test9():

    with_def=1  # set to 1 to test with define_columns

    config_list=[]
    i=1
    while i < min(len(sys.argv),9):
        config_list.append(int(sys.argv[i]))
        i=i+1

    if len(sys.argv) > 9:
        try:
            lastarg=int(sys.argv[9])
            config_list.append(lastarg)
            shpfile=None
        except:
            ##traceback.print_exc()
            shpfile=sys.argv[9]
            config_list.append(None)
    else:
        shpfile=None
        config_list.append(None)

    cfg=tuple(config_list)

    if len(sys.argv) == 11:
        shpfile=sys.argv[10]

    cfgstr=str(cfg)
    emptylist=[]

    win10=pguTestGridWin('Test grid 9: emptylist '+cfgstr,config=cfg)
    win10.set_source([])
    win10.grid.set_empty_message(None)
    win10.move(20,100)

    win10.grid.resize_to_default()
    gtk.main()


def test1b():
    with_def=1  # set to 1 to test with define_columns

    config_list=[]
    i=1
    while i < min(len(sys.argv),9):
        config_list.append(int(sys.argv[i]))
        i=i+1

    if len(sys.argv) > 9:
        try:
            lastarg=int(sys.argv[9])
            config_list.append(lastarg)
            shpfile=None
        except:
            ##traceback.print_exc()
            shpfile=sys.argv[9]
            config_list.append(None)
    else:
        shpfile=None
        config_list.append(None)

    cfg=tuple(config_list)

    if len(sys.argv) == 11:
        shpfile=sys.argv[10]

    cfgstr=str(cfg)
    emptylist=[]

    p1=['test1','test2','test3','test4','test5']
    p2=[11.5,1.5,1.2,1.66,5.4]
    shps2=gview.GvShapes()
    for idx in range(len(p2)):
        shps2.append(gview.GvShape())
        shps2[idx].set_node(5,3)
        shps2[idx].set_property('prop1',str(p1[idx]))
        shps2[idx].set_property('prop2',str(p2[idx]))

    win4 = pguTestGridWin('Test grid 1b: shps without schema, '+cfgstr,
                          config=cfg,source=shps2)
    if with_def == 1:
        win4.grid.define_columns(members=['prop2'],
                         titles=['FLOAT title'],
                         editables=None,
                         types=['float'],justify=1,title_justify=1)
        print 'Test grid 1b column titles: all left'
        print 'Test grid 1b columns      : all left'

    win4.move(20,300)
    win4.grid.set_default_selection_colour((0,0,65000))

    ##win4.grid.resize_to_default()

    gtk.main()

def test():

    import sys

    with_def=1  # set to 1 to test with define_columns

    config_list=[]
    i=1
    while i < min(len(sys.argv),9):
        config_list.append(int(sys.argv[i]))
        i=i+1

    if len(sys.argv) > 9:
        try:
            lastarg=int(sys.argv[9])
            config_list.append(lastarg)
            shpfile=None
        except:
            ##traceback.print_exc()
            shpfile=sys.argv[9]
            config_list.append(None)
    else:
        shpfile=None
        config_list.append(None)

    cfg=tuple(config_list)

    if len(sys.argv) == 11:
        shpfile=sys.argv[10]

    cfgstr=str(cfg)
    p1=['test1','test2','test3','test4','test5']
    p2=[11.5,1.5,1.2,1.66,5.4]
    p3=[1,2,3,4,600]
    shps=gview.GvShapes()
    shps.add_field('prop1-string','string',10)
    shps.add_field('prop2-float','float',10,5)
    shps.add_field('prop3-int','integer',10)
    for idx in range(30):
        shps.append(gview.GvShape())
        shps[idx].set_node(5,3)
        shps[idx].set_property('prop1-string',str(p1[idx%5]))
        shps[idx].set_property('prop2-float',str(p2[idx%5]))
        shps[idx].set_property('prop3-int',str(p3[idx%5]))

    win = pguTestGridWin('Test grid 1: shps with schema, '+cfgstr,config=cfg, source=shps)
    ###win.set_source(shps)

    if with_def == 1:
        win.grid.define_columns(members=['prop1-string','prop2-float',
                                         'prop3-int'],
                            titles=['STRING title','FLOAT title','INT title'],
                            editables=None,
                            formats=["%-s","%-10.3f","%3d"],
                            types=['string','float','integer'],
                            justify=[0,1,2],title_justify=[0,1,2])
        print 'Test grid 1 column titles: right, left, center'
        print 'Test grid 1 columns      : right, left, center'

    idx=win.grid.add_style(((65000,0,0),None,(65000,30000,30000),None,None,None))
    idx2=win.grid.add_style(((0,0,65000),None,(30000,30000,65000),None,None,None))
    win.grid.set_row_style([1,2,3,4,5],idx)
    win.grid.set_column_style([0,2],idx2)
    win.grid.set_default_style(((65000,65000,0),None,(0,65000,65000),None,None,None))
    win.connect( 'delete-event', gtk.main_quit )
    win.move(20,20)

    win2=pguTestGridWin('Test grid 2: Numpy, '+cfgstr,config=cfg)
    numpy=Numeric.array([[1,2,3,4,5,6,7,8,9,0,1,2],[2,3,4,5,6,6,7,8,9,0,1,2],
                         [6,5,4,3,2,6,7,8,9,0,1,2],[1,3,5,7,9,6,7,8,9,0,1,2]])
    win2.set_source(numpy)
    if cfg[0] in [1,3]:
        win2.grid.set_row_title_type('source',expose=0)

    if with_def == 1:    
        win2.grid.define_columns(members=[0,1,2,3,4,5,6,7,8,9,10,11],
                            titles=['c0','c1','c2','c3','c4','Column Five',
                                    'c6','Seventh Column with Long Title',
                                    'c8','c9',
                                    'c10- yet another very long title',
    'c11-really,really,really,really,really,really,really,really long title'],
                            editables=None,
                            types='float',
                            justify=1,title_justify=0,force_width=90)
        print 'Test grid 2 column titles: all right'
        print 'Test grid 2 columns      : all left'


    win2.move(400,20)
    idx=win2.grid.add_style(((65000,0,65000),None,(65000,30000,65000),None,None,None))
    idx2=win2.grid.add_style(((0,65000,0),None,(30000,65000,30000),None,None,None))
    win2.grid.set_row_style([1,2],idx)
    win2.grid.set_column_style([0,1,2],idx2)
    win2.grid.set_default_row_title_style(((0,0,45000),None,None,None,None,None))
    win2.grid.set_default_col_title_style(((65000,40000,10000),None,None,None,None,None))
    listlist=[[1,'hello',1.2],[2,'hi',3.4],[3,'bonjour',5.6],
              [4,'hola',3.2],[5,'guten tag',1.4]]

    win2.grid.set_scroll_policy(1,1)

    win3 = pguTestGridWin('Test grid 3: list of lists '+cfgstr,config=cfg)
    if with_def == 1:    
        win3.grid.define_columns(members=[1,0,2,1],
                        titles=['c11','cl0','c12','c11again'],
                                 justify=0,title_justify=2,
                                 force_width=50)
        print 'Test grid 3 column titles: all center'
        print 'Test grid 3 columns      : all right'

    win3.set_source(listlist)
    win3.move(800,20)

    shps2=gview.GvShapes()
    for idx in range(5):
        shps2.append(gview.GvShape())
        shps2[idx].set_node(5,3)
        shps2[idx].set_property('prop1',str(p1[idx]))
        shps2[idx].set_property('prop2',str(p2[idx]))
    print "1b", "Z"*50
    win4 = pguTestGridWin('Test grid 1b: shps without schema, '+cfgstr,
                          config=cfg,source=shps2)
    if with_def == 1:
        win4.grid.define_columns(members=['prop2'],
                         titles=['FLOAT title'],
                         editables=None,
                         types=['float'],justify=1,title_justify=1)
        print 'Test grid 1b column titles: all left'
        print 'Test grid 1b columns      : all left'

    win4.move(20,300)
    win4.grid.set_default_selection_colour((0,0,65000))

    listobj=[]
    listobj.append(_test_listobj('obj0',0,0.1,complex(1,-1)))
    listobj.append(_test_listobj('obj1',2,0.2,complex(2,-1)))
    listobj.append(_test_listobj('obj2',4,3.7,complex(3,1)))
    listobj.append(_test_listobj('obj3',1,2.0,complex(0,-5)))
    listobj.append(_test_listobj('obj4',3,0.1,complex(1,0)))
    win5 = pguTestGridWin('Test grid 4: list of objects '+cfgstr,config=cfg)
    if with_def == 1:
        win5.grid.define_columns(members=['stringval','intval'],justify=[1,2],
                                 title_justify=[0,2])
        print 'Test grid 4 column titles: right, center'
        print 'Test grid 4 columns      : left,center'
        win5.grid.set_source(listobj,redefine_columns=0)
        win5.grid.resize_to_default()
    else:
        win5.set_source(listobj)

    win5.move(400,300)


    if with_def == 1:
        win.grid.resize_to_default()
        win2.grid.resize_to_default()
        win3.grid.resize_to_default()
        win4.grid.resize_to_default()
        win5.grid.resize_to_default()

    # If shapefile specified
    print 'shpfile: ',shpfile
    if shpfile is not None:
        shps6=gview.GvShapes(shapefilename=shpfile)
        win6 = pguTestGridWin('Test grid 5: shapefile '+cfgstr,config=cfg,
                          source=shps6)
        #if with_def == 1:
        #    win6.grid.define_columns(formats=[None,None,'%11.2f','%11.2f'])

        win6.move(800,300)
        win6.grid.resize_to_default()
        print 'Test grid 5 column titles: all center'
        print 'Test grid 5 columns      : all right'

    listobj2=[1,2,3,4,5]
    win7=pguTestGridWin('Test grid 6: list of integers '+cfgstr,config=cfg)
    win7.set_source(listobj2)
    win7.move(20,600)

    listobj2=('one','two','three','four','five','six','seven')
    win8=pguTestGridWin('Test grid 7: tuple of strings '+cfgstr,config=cfg)
    win8.set_source(listobj2)
    win8.move(400,600)


    listobj2=((1,'hello',3,5.2),(8,'hi',7,9.2),(64,'hola',1,3.8))
    win9=pguTestGridWin('Test grid 8: tuple of tuples '+cfgstr,config=cfg)
    win9.move(800,600)
    win9.grid.set_scroll_policy(1,1)
    win9.set_source(listobj2)

    emptylist=[]
    win10=pguTestGridWin('Test grid 9: emptylist '+cfgstr,config=cfg)
    win10.set_source(emptylist)
    win10.grid.set_empty_message(None)
    win10.move(20,800)

    win11=pguTestGridWin('Test grid 10: titled emptylist '+cfgstr,config=cfg)
    win11.grid.set_source(emptylist,
                          members=(1,2,3),titles=('one','two','three'))
    win11.move(400,800)

    win12=pguTestGridWin('Test grid 10: titled None '+cfgstr,config=cfg)
    win12.grid.set_source(emptylist,members=('col one','col two','col three'))
    win12.move(800,800)

    win.grid.set_line_drawing(2,2)

    if with_def == 1:
        win7.grid.resize_to_default()
        win8.grid.resize_to_default()
        win9.grid.resize_to_default()
        win10.grid.resize_to_default()
        win11.grid.resize_to_default()
        win12.grid.resize_to_default()


    gtk.main()


if __name__ == '__main__':
    test()


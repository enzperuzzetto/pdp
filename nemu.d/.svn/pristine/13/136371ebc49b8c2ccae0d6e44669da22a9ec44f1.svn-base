# ------------------------------------------------------
# -- NEmu : The Network Emulator for Mobile Universes --
# ------------------------------------------------------

# Copyright (C) 2011-2016  Vincent Autefage

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.

#    You should have received a copy of the GNU Lesser General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# http://nemu.valab.net

'''This module manages NEmu session history'''

import sys
import os
from nemu.var import NemuVar
from nemu.msg import (printc,printlog)
from nemu.error import NemuError

def save_nemu_history(line, dest=None):
    '''Internal history saving handler'''
    _dest = dest
    if dest == None:
        _dest = NemuVar.histf
    if os.path.isfile(_dest):
        printlog(line.rstrip("\r\n"), logf=_dest)

def restore_nemu_history(src=None):
    '''Internal history restoring handler'''
    _src = src
    if src == None:
        _src = NemuVar.histf
    if sys.modules.has_key('readline') and os.path.isfile(_src):
        import readline
        printc('Reading NEmu history from ' + _src, 'cyan')
        with open(_src, 'r') as fd:
            for line in fd:
                readline.add_history(line.split('\t')[-1].rstrip("\r\n"))

def HistoryNemu(dest=None):
    '''HistoryNemu(dest)
    - Def -
        Displays the command history of the current session.
    - Args -
        . dest [str] --> the output filename or None for stdout (Default: None)
    '''
    if sys.modules.has_key('readline'):
        import readline
        target = dest
        if not target:
            for i in range(1, readline.get_current_history_length()):
                print(readline.get_history_item(i))
        else:
            if not isinstance(dest, str):
                raise NemuError('Cannot write in ' + str(dest))
            save_nemu_history(target)

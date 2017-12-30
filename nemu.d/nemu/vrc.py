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

'''This module is designed to create init plugins script for VRouter services'''

from nemu.msg import printc_common
from nemu.path import mpath

class VRc:
    '''VRc(name, id, vrouter, mode)
    - Def -
        Generates a new plugin init script.
    - Args -
        . name [str] --> script name
        . id [int] --> script id
        . vrouter [vrouter] --> vrouter
        . mode [str] --> script open mode w, r, rw, a (default: w)
    '''
    def __init__(self, name, id, vrouter, mode='w'):
        self.fd = open(mpath(vrouter.initd, str(id) + '_' + str(name)), mode)
        printc_common('#!/bin/sh', None, self.fd)
        
    def write(self, com):
        '''Writes <com> into the shell script'''
        printc_common(com, None, self.fd)

    def close(self):
        '''Closes the shell script'''
        self.fd.close()

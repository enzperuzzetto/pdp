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

import os
import shutil
from nemu.vrc import VRc
from nemu.path import mpath

def init(vrouter, *largs, **kargs):
    for f in largs:
        if os.path.isfile(f):
            dest = mpath(vrouter.files, os.path.basename(f))
            if not os.path.exists(dest):
                shutil.copyfile(f, dest)
        elif os.path.isdir(f):
            dest = mpath(vrouter.files, os.path.basename(f))
            if not os.path.exists(dest):
                shutil.copytree(f, dest)

def help():
    ret = dict()
    ret['syn'] = 'Service("file", ...)'
    ret['desc'] = 'Imports additional user files'
    ret['args'] = list()
    ret['args'].append('[list of strings] : files path')
    return ret

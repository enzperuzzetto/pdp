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

def _import(vrouter, dir, mod):
    for f in os.listdir(dir):
        if f.find(mod) >= 0:
            shutil.copyfile(mpath(dir, f), mpath(vrouter.tcz, f))
    dep = mpath(dir, mod + '.dep')
    if os.path.isfile(dep):
        fd = open(dep, 'r')
        for line in fd.readlines():
            if line.find('.tcz') < 0:
		continue
	    if 'KERNEL' in line:
		line = line.replace('KERNEL', __version_vrouter_kernel__)
            _import(vrouter, dir, line.rstrip("\r\n").lstrip("\r"))

def init(vrouter, *largs, **kargs):
    root = os.getcwd()
    if 'root' in kargs:
        root = kargs['root']
    for i in largs:
        mod = i
        if root:
            mod = mpath(root, mod)
        dir = os.path.dirname(mod)
        mod = os.path.basename(mod) + '.tcz'
        _import(vrouter, dir, mod)
        
        
def help():
    ret = dict()
    ret['syn'] = 'Service("tcz", ... , root)'
    ret['desc'] = 'Loads additional TCZ packages at startup'
    ret['args'] = list()
    ret['args'].append('[list of strings] : packages name')
    ret['args'].append('root [string] : root directory of packages (Default: None)')
    return ret

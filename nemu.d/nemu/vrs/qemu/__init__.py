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

from nemu.vrc import VRc

def init(vrouter, *largs, **kargs):
    for i in largs:
        vrouter.opts[i.replace('_', '-')] = None
    for k in kargs.keys():
        vrouter.opts[k.replace('_', '-')] = kargs[k]

def help():
    ret = dict()
    ret['syn'] = 'Service("qemu", ... , ...)'
    ret['desc'] = 'Enables several qemu features'
    ret['args'] = list()
    ret['args'].append('[list of strings] : qemu simple options')
    ret['args'].append('[list of dict] : qemu tuple options')
    return ret

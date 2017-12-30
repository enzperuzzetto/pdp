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
    fd = VRc(name='self_script', id=str(kargs['id']), vrouter=vrouter)
    for i in largs:
        ofd = open(i, 'r')
        for line in ofd.readlines():
            fd.write(line)
        ofd.close()
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("script", ... ,id)'
    ret['desc'] = 'Shell scripts to execute at startup'
    ret['args'] = list()
    ret['args'].append('[list of strings] : scripts list')
    ret['args'].append('id [int] : ID of the generated script')
    return ret

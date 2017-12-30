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
from nemu.path import mpath

def init(vrouter, *largs, **kargs):
    fd = VRc(name='mount', id='00', vrouter=vrouter, mode='a+')
    part = ''
    mountdir = '/mnt'
    dev = vrouter.hds[int(kargs['hd'])]
    devid = chr(ord('a') + int(kargs['hd']))
    tag = dev.tag
    options = ''
    if 'part' in kargs:
        part = str(kargs['part'])
    if 'root' in kargs:
        mountdir = str(kargs['root'])
    if 'tag' in kargs:
        tag = str(kargs['tag'])
    if len(largs) > 0:
        options = '-o '
        for i in largs:
            options += str(i) + ','
        options = options.rstrip(',')
    mountdir = mpath(mountdir, tag)
    fd.write('mkdir -p ' + mountdir)
    if dev.type == 'virtio':
        fd.write('mount.virtio ' + tag + ' ' + mountdir)
    else:
        fd.write('mount ' + options + ' /dev/sd' + devid + part + ' ' + mountdir)
    fd.close()


def help():
    ret = dict()
    ret['syn'] = 'Service("mount", ... , hd, part, root, tag)'
    ret['desc'] = 'Mounts a device at startup'
    ret['args'] = list()
    ret['args'].append('[list of strings] : mount options')
    ret['args'].append('hd [int] : device number to mount')
    ret['args'].append('part [int] : device partition to mount (Default: empty)')
    ret['args'].append('root [string] : mount root directory (Default: /mnt)')
    ret['args'].append('tag [string] : mount tag (Default: VFs tag)')
    return ret

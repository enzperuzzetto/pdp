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

'''This module contains functions for exporting NEmu session configuration'''

from nemu.vnode import VNode
from nemu.link import Link
from nemu.vlink import VLink
from nemu.var import NemuVar
from nemu.msg import (printc, printok, printfd)
from nemu.remote import RemoteNemu
from nemu.nbd import RemoteNBD
from nemu.join import Join
from nemu.mobile import MobNemu


def ExportNemu(dest=None):
    '''ExportNemu(dest)
    - Def -
        Writes the current NEmu session topology in a text file or on stdout.
    - Args -
        . dest [str] --> output filename (Default: None)
    '''
    if not NemuVar.this:
        return
    fd = None
    if dest:
        printc('Exporting NEmu configuration to ' + str(dest), 'blue')
        fd = open(dest, 'w')
    com = str(NemuVar.this)
    printfd(com, fd)
    keys = sorted(RemoteNBD.set.keys())
    for i in keys:
        this = RemoteNBD.set[i]
        com = str(this)
        printfd(com, fd)
    keys = sorted(VNode.set.keys())
    for i in keys:
        this = VNode.set[i]
        com = str(this)
        printfd(com, fd)
    keys = sorted(VLink.set.keys())
    for i in keys:
        this = VLink.set[i]
        com = str(this)
        printfd(com, fd)
    keys = sorted(VNode.set.keys())
    for i in keys:
        this = VNode.set[i]
        for iface in this.ifaces:
            com = str(iface)
            if len(com) > 0:
                printfd(com, fd)
    keys = sorted(VLink.set.keys())
    for i in keys:
        this = VLink.set[i]
        com = ''
        for iface in this.ifaces:
            com = str(iface)
            if len(com) > 0:
                printfd(com, fd)
    keys = sorted(Link.set.keys())
    for i in keys:
        this = Link.set[i]
        for iface in this:
            if Link.set[i][iface] and Link.set[i][iface].client.name == i:
                com = str(Link.set[i][iface])
                printfd(com, fd)
    keys = sorted(Join.set.keys())
    for i in keys:
        this = Join.set[i]
        for iface in this:
            if Join.set[i][iface] and Join.set[i][iface].client.name == i:
                com = str(Join.set[i][iface])
                printfd(com, fd)
    keys = sorted(MobNemu.set.keys())
    for i in keys:
        this = MobNemu.set[i]
        com = str(this)
        printfd(com, fd)
    for i in RemoteNemu.set.keys():
        this = RemoteNemu.set[i]
        com = str(this)
        printfd(com, fd)
        ret = this.rsend('ExportNemu()')
        for line in ret:
            com = 'ComRemoteNemu(' + i + ',"' + line.rstrip('\n\r') + '")'
            printfd(com, fd)
    if not dest == None:
        fd.close()
        printok()

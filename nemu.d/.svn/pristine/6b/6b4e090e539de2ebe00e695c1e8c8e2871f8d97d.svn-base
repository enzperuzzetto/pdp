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

'''This module manages NEmu graph visualization'''

from nemu.error import NemuError
from nemu.vnode import VNode
from nemu.link import Link
from nemu.vlink import VLink
from nemu.var import NemuVar
from nemu.msg import (printc, printok, printc_common)
from nemu.proc import actproc    
from nemu.remote import (RemoteNemu, isremoted)
from nemu.join import Join

def GraphNemu(dest, type='dot', layout='neato'):
    '''GraphNemu(dest, type, layout)
    - Def -
        Generates a graph of the current NEmu session topology with Graphviz.
    - Args -
        . dest [str] --> the output filename
        . type [str] --> dot compatible format (png, pdf,...) (Default: dot)
        . layout [str] --> available layout (dot, neato, circo, twopi, fdp) (Default: neato)
    '''
    if not NemuVar.this:
        return
    if not dest:
        raise NemuError('Cannot write in ' + str(dest))
    ddest = NemuVar.dotf
    fd = open(ddest, 'w')
    com = 'graph ' + 'NEmu' + ' {'
    printc_common(com, None, fd)
    com = 'overlap=scale;'
    printc_common(com, None, fd)
    com = 'splines=true;'
    printc_common(com, None, fd)
    com = 'nodesep=2.0;'
    printc_common(com, None, fd)
    com = 'label="NEmu";'
    printc_common(com, None, fd)
    for i in SubGraphNemu():
        printc_common(i, None, fd)
    com = '}'
    printc_common(com, None, fd)
    fd.close()
    printc('Exporting NEmu graph to ' + str(dest) + ' in ' + str(type) + ' with the layout ' + str(layout), 'blue')
    com = str(layout) + ' -T' + str(type) + ' ' + str(ddest) + ' -o ' + str(dest)
    actproc(com)
    printok()

def SubGraphNemu():
    '''internal nemu dot script generator'''
    ret = list()
    if not NemuVar.this:
        return ret
    com = 'subgraph "' + NemuVar.session + '" {'
    ret.append(com)
    com = 'overlap=scale;'
    ret.append(com)
    com = 'splines=true;'
    ret.append(com)
    com = 'nodesep=2.0;'
    ret.append(com)
    com = 'label="' + NemuVar.session + '";'
    ret.append(com)
    keys = sorted(VNode.set.keys())
    for i in keys:
        this = VNode.set[i]
        com = this.gstr()
        ret.append(com)
    keys = sorted(VLink.set.keys())
    for i in keys:
        this = VLink.set[i]
        com = this.gstr()
        ret.append(com)
    keys = sorted(Link.set.keys())
    for i in keys:
        this = Link.set[i]
        for iface in this:
            if Link.set[i][iface] and Link.set[i][iface].client.name == i:
                com = Link.set[i][iface].gstr()
                ret.append(com)
    keys = sorted(Join.set.keys())
    for i in keys:
        this = Join.set[i]
        for iface in this:
            if Join.set[i][iface] and Join.set[i][iface].client.name == i:
                com = Join.set[i][iface].gstr()
                ret.append(com)
    com = '}'
    ret.append(com)
    for i in RemoteNemu.set.keys():
        this = RemoteNemu.set[i]
        rret = this.rsend('SubGraphNemu()')
        for j in rret:
            com = str(i) + '::' + j
            ret.append(j)
    if isremoted():
        for line in ret:
            print(line.rstrip("\n"))
    return ret

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

'''This module manages the delete of virtual elements'''

import os
import shutil

from nemu.error import NemuError
from nemu.link import (Link, Unlink)
from nemu.vnode import VNode
from nemu.vlink import VLink
from nemu.vnd import (VLine, VHub, VSwitch)
from nemu.vde import (VDEHub, VDESwitch)
from nemu.vremote import VRemote
from nemu.vslirp import VSlirp
from nemu.vqhub import VQHub
from nemu.vhost import (VHostConf, VHost)
from nemu.vrouter import (VRouterConf, VRouter)
from nemu.msg import (printc, printok)
from nemu.proc import killprocl
from nemu.var import (NemuVar, _NemuVar)
from nemu.name import NemuNameSpace
from nemu.remote import (RemoteNemu, DelRemoteNemu)
from nemu.nbd import (RemoteNBD, DelRemoteNBD)
from nemu.vair import (VAirLink, VAirWic, VAirAp, VAirRemote)
from nemu.join import (Join, Unjoin)
from nemu.mobile import (MobNemu, DelMobNemu)

def DelVNode(*names):
    '''DelVNode(*names)
    - Def -
        Deletes some virtual nodes.
    - Args -
        . names [list of str] --> names of the desired virtual nodes (Default: deletes all)
    '''
    hlist = list(names)
    if len(names) == 0:
        hlist = sorted(VNode.set.keys())
    for i in hlist:
        try:
            node = VNode.set[i]
            Unlink(node.name)
            node.delete()
        except KeyError:
            raise NemuError('Cannot find the Virtual Node ' + str(i))

def DelVLink(*names):
    '''DelVLink(*names)
    - Def -
        Deletes some virtual links.
    - Args -
        . names [list of str] --> names of the desired virtual links (Default: deletes all)
    '''
    hlist = list(names)
    if len(names) == 0:
        hlist = sorted(VLink.set.keys())
    for i in hlist:
        try:
            net = VLink.set[i]
            Unlink(net.name)
            Unjoin(net.name)
            net.delete()
        except KeyError:
            raise NemuError('Cannot find the Virtual Link ' + str(i))


def DelNemu():
    '''DelNemu()
    - Def -
        Deletes the current NEmu session.
    '''
    printc('Deleting the current NEmu session', 'blue')
    if not NemuVar.rootd == '' and os.path.isdir(NemuVar.rootd) and not NemuVar.lnf == '':
        if os.path.islink(NemuVar.lnf):
            os.unlink(NemuVar.lnf)
        for i in MobNemu.set.keys():
            DelMobNemu(i)
        for i in VNode.set.keys():
            DelVNode(i)
        for i in VLink.set.keys():
            DelVLink(i)
        for i in RemoteNemu.set.keys():
            DelRemoteNemu(i)
        for i in RemoteNBD.set.keys():
            DelRemoteNBD(i)
        killprocl()
        shutil.rmtree(NemuVar.rootd)
    else:
        killprocl()
        raise NemuError('Illegal session')
    VLink.set = dict()
    VLine.set = dict()
    VHub.set = dict()
    VSwitch.set = dict()
    VRemote.set = dict()
    VSlirp.set = dict()
    VQHub.set = dict()
    VDEHub.set = dict()
    VDESwitch.set = dict()
    VAirLink.set = dict()
    VAirWic.set = dict()
    VAirAp.set = dict()
    VAirRemote.set = dict()
    Join.set = dict()
    VNode.set = dict()
    VRouter.set = dict()
    VRouterConf.set = dict()
    VHost.set = dict()
    VHostConf.set = dict()
    Link.set = dict()
    RemoteNemu.set = dict()
    RemoteNBD.set = dict()
    MobNemu.set = dict()
    NemuNameSpace.set = dict()
    _NemuVar()
    printok()

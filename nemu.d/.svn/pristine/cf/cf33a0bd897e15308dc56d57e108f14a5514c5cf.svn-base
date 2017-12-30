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

'''This module manages session startup and halting'''

from nemu.error import NemuError
from nemu.link import Link
from nemu.vlink import VLink
from nemu.vnode import VNode
from nemu.msg import (printc, printok)
from nemu.proc import (waitprocl, NemuProc)

def StartVNode(*names):
    '''StartVNode(*names)
    - Def -
        Starts some virtual nodes.
    - Args -
        . names [list of str] --> names of the desired virtual nodes (Default: starts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VNode.set.keys())
    for i in hlist:
        try:
            node = VNode.set[i]
        except KeyError:
            raise NemuError('Cannot find the Virtual Node ' + str(i))
        node.start()
        if node.name in Link.set:
            for link in Link.set[node.name].values():
                if link:
                    link.start()

def StartVLink(*names):
    '''StartVLink(*names)
    - Def -
        Starts some birtual links.
    - Args -
        . names [list of str] --> names of the desired virtual links (Default: starts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VLink.set.keys())
    for i in hlist:
        try:
            net = VLink.set[i]
        except KeyError:
            raise NemuError('Cannot find the Virtual Link ' + str(i))
        net.start()
        if net.name in Link.set:
            for link in Link.set[net.name].values():
                if link:
                    link.start()
        
def StopVNode(*names):
    '''StopVNode(*names)
    - Def -
        Stops some virtual nodes.
    - Args -
        . names [list of str] --> names of the desired virtual nodes (Default: halts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VNode.set.keys())
    for i in hlist:
        try:
            node = VNode.set[i]
            if node.name in Link.set:
                for link in Link.set[node.name].values():
                    if link:
                        link.stop()
            node.stop()
        except KeyError:
            raise NemuError('Cannot find the Virtual Node ' + str(i))

def StopVLink(*names):
    '''StopVLink(*names)
    - Def -
        Stops some virtual links.
    - Args -
        . names [list of str] --> names of the desired virtual links. (Default: halts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VLink.set.keys())
    for i in hlist:
        try:
            net = VLink.set[i]
            if net.name in Link.set:
                for link in Link.set[net.name].values():
                    if link:
                        link.stop()
            net.stop()
        except KeyError:
            raise NemuError('Cannot find the Virtual Link ' + str(i))


def RebootVNode(*names):
    '''RebootVNode(*names)
    - Def -
        Reboots some virtual nodes.
    - Args -
        . names [list of str] --> names of the desired virtual nodes (Default: restarts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VNode.set.keys())
    for i in hlist:
        try:
            node = VNode.set[i]
            node.reboot()
        except KeyError:
            raise NemuError('Cannot find the Virtual Node ' + str(i))

def RebootVLink(*names):
    '''RbootVLink(*names)
    - Def -
        Reboots some virtual links.
    - Args -
        . names [list of str] --> names of the desired virtual links (Default: restarts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VLink.set.keys())
    for i in hlist:
        StopVLink(i)
        StartVLink(i)

def WaitNemu(*names):
    '''WaitNemu(*names)
    - Def -
        Waits for several virtual nodes or links end of life.
    - Args -
        . names [list of str] --> names of the desired virtual nodes or links
    '''
    plist = list(names)
    procs = list()
    for i in plist:
        try:
            node = None
            if i in VNode.set:
                node = VNode.set[i]
            else:
                node = VLink.set[i]
            if not node.running():
                continue
            if node.proc and node.proc.pid in NemuProc.procl:
                procs.append(node.proc.pid)
        except KeyError:
            raise NemuError('Cannot find ' + str(i))
    printc('Waiting for process ending...', 'blue')
    waitprocl(*procs)
    printok()

def StartNemu():
    '''StartNemu()
    - Def -
        Starts virtual nodes and links.
    '''
    StartVLink()
    StartVNode()

def StopNemu():
    '''StopNemu()
    - Def -
        Stops virtual nodes and links.
    '''
    StopVNode()
    StopVLink()

def RebootNemu():
    '''RebootNemu()
    - Def -
        Restarts virtual nodes and links.
    '''
    StopNemu()
    StartNemu()

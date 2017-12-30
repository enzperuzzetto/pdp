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

'''This module manages virtual hosts'''

from nemu.name import NemuNameSpace
from nemu.var import NemuVar
from nemu.error import NemuError
from nemu.msg import (printc, printok)
from nemu.vnode import VNode

class VHostConf():
    '''VHostConf(name, qemu, **opts)
    - Def -
        A VHostConf is a common configuration for several VHosts.
    - Args -
        . name [str] --> VHostConf name
        . qemu [str] --> QEMU executable (Default: qemu)
        . opts [dict] --> other qemu options. a=1, b_c="x", d=["y","z"] -> qemu -a 1 -b-c x -d y -d z... (Default: empty)
    '''
    set = dict()
    def __init__(self, name, qemu=NemuVar.qemu, **opts):
        printc('Setting up VHostConf ' + str(name), 'blue')
        if name in VHostConf.set:
            raise NemuError('VHostConf ' + str(name) + ' already exists')
        if name in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        self.name = name
        self.qemu = qemu
        self.opts = dict()
        for i in opts.keys():
            self.opts[i.replace('_', '-')] = opts[i]
        VHostConf.set[name] = self
        NemuNameSpace.set[name] = self
        printok()
        
    def delete(self):
        '''Deletes the vnode configuration'''
        printc('Deleting the VHostConf ' + str(self.name) , 'blue')
        del VHostConf.set[self.name]
        del NemuNameSpace.set[self.name]
        printok()
        

class VHost(VNode):
    '''VHost(name, conf, hds, nics, qemu, **opts)
    - Def -
        A VHost is a QEMU virtual host.
    - Args -
        . name [str] --> name of the desired host
        . conf [str] --> VHostConf name (Default: None)
        . hds [list of VFs] --> hard drives list (Default: empty)
        . nics [list of VNic] --> network interface cards (Default: empty)
        . qemu [str] --> QEMU executable (Default: qemu)
        . opts [dict] --> other qemu options. a=1, b_c="x", d=["y","z"] -> qemu -a 1 -b-c x -d y -d z... (Default: empty)
    '''
    set = dict()
    def __init__(self, name, conf=None, hds=list(), nics=list(), qemu=None, **opts):
        self.name = name
        self.hds = hds
        self.ifaces = nics
        self.qemu = qemu
        self.opts = dict()
        if not conf == None:
            printc('Setting up VHost ' + str(name) + ' with the VHostConf ' + str(conf), 'blue')
            try:
                self.conf = VHostConf.set[conf]
            except KeyError:
                raise NemuError('Cannot find the VHostConf ' + str(conf))
            if qemu == None:
                self.qemu = self.conf.qemu
            self.opts.update(self.conf.opts)
        else:
            printc('Setting up VHost ' + str(name), 'blue')
        self.opts.update(opts)
        VNode.__init__(self, self.name, self.hds, self.ifaces, self.qemu, **self.opts)
        VHost.set[name] = self
        printok()

    def start(self):
        VNode.start(self)
        
    def stop(self):
        if self.running():
            VNode.stop(self)

    def delete(self):
        printc('Deleting the VHost ' + str(self.name) , 'blue')
        self.stop()
        del VHost.set[self.name]
        VNode.delete(self)
        printok()
        
    def __str__(self):
        return VNode.__str__(self)

    def __repr__(self):
        return str(self)

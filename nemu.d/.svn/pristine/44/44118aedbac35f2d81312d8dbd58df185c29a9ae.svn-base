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

'''This module contains functions for VRouter'''

import os
import shutil

from nemu.name import NemuNameSpace
from nemu.var import NemuVar
from nemu.error import NemuError
from nemu.msg import (printc, printok, printstr)
from nemu.path import mpath
from nemu.var import NemuVar
from nemu.vfs import VFs
from nemu.vnode import VNode
from nemu.service import Service

class VRouterConf():
    '''VRouterConf(name, services, **opts)
    - Def -
        A VRouterConf is a common configuration for several VRouters.
    - Args -
        . name [str] --> VRouterConf name
        . services [list of Service] --> enables router services (default: empty)
        . opts [dict] --> other qemu options. a=1, b_c="x", d=["y","z"] -> qemu -a 1 -b-c x -d y -d z... (Default: empty)
    '''
    set = dict()
    def __init__(self, name, services=list(), **opts):
        printc('Setting up VRouterConf ' + str(name), 'blue')
        if name in VRouterConf.set:
            raise NemuError('VRouterConf ' + str(name) + ' already exists')
        if name in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        self.name = name
        self.services = list()
        self.opts = dict()
        for i in services:
            if not isinstance(i, Service):
                raise NemuError(str(i) + ' is not a Service')
            self.services.append(i)
        for i in opts.keys():
            self.opts[i.replace('_', '-')] = opts[i]
        VRouterConf.set[name] = self
        NemuNameSpace.set[name] = self
        printok()

    def delete(self):
        '''Deletes the vrouter configuration'''
        printc('Deleting the VRouterConf ' + str(self.name) , 'blue')
        del VRouterConf.set[self.name]
        del NemuNameSpace.set[self.name]
        printok()


class VRouter(VNode):
    '''VRouter(name, conf, hds, nics, services, **opts)
    - Def -
        A VRouter is a virtual router created by NEmu in order to offer several useful services.
    - Args -
        . name [str] --> VRouter name
        . conf [str] --> VRouterConf name (Default: None)
        . hds [list of VFs] --> hard drives list (Default: empty)
        . nics [list of VNic] --> network interface cards list (Default: empty)
        . services [list of Service] --> enables router services (default: empty)
        . opts [dict] --> other qemu options. a=1, b_c="x", d=["y","z"] -> qemu -a 1 -b-c x -d y -d z... (Default: empty)
    '''
    set = dict()
    def __init__(self, name, conf=None, hds=list(), nics=list(), services=list(), **opts):
        self.services = list()
        self.opts = dict()
        if not conf == None:
            printc('Setting up VRouter ' + str(name) + ' with the VRouterConf ' + str(conf), 'blue')
            try:
                self.conf = VRouterConf.set[conf]
            except KeyError:
                raise NemuError('Cannot find the VRouterConf ' + str(conf))
            self.services.extend(self.conf.services)
            self.opts.update(self.conf.opts)
        else:
            printc('Setting up VRouter ' + str(name), 'blue')
        if name in VRouter.set:
            raise NemuError('VRouter ' + str(name) + ' already exists')
        if name in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        for i in services:
            if not isinstance(i, Service):
                raise NemuError(str(i) + ' is not a Service')
            self.services.append(i)
        self.name = name
        self.rcd = mpath(NemuVar.rcd, self.name)
        self.kernel = mpath(NemuVar.rcd, NemuVar.rcdkernel)
        self.initd = mpath(self.rcd, NemuVar.rcdinitd)
        self.initrd = mpath(NemuVar.rcd, NemuVar.rcdinitrd)
        self.tcz = mpath(self.rcd, NemuVar.rcdtcz)
        self.files = mpath(self.rcd, NemuVar.rcdfiles)
        self.boot = mpath(NemuVar.rcd, NemuVar.rcdboot)
        self.rfs = mpath(NemuVar.fsd, str(self.name) + '.img')
        if not os.path.isdir(self.rcd):
            os.mkdir(self.rcd)
        if not os.path.isdir(self.initd):
            os.mkdir(self.initd)
        if not os.path.isdir(self.tcz):
            os.mkdir(self.tcz)
        if not os.path.isdir(self.files):
            os.mkdir(self.files)
        self.opts.update(opts)
        self.services.append(Service('hostname'))
        VNode.__init__(self, self.name, hds=hds, nics=nics, qemu=NemuVar.qemu, **self.opts)
        VRouter.set[name] = self
        printok()
        
    def clean(self):
        '''Cleans the root file system of the router'''
        if os.path.isdir(self.initd):
            shutil.rmtree(self.initd, True)
            os.mkdir(self.initd)
        if os.path.isdir(self.tcz):
            shutil.rmtree(self.tcz, True)
            os.mkdir(self.tcz)
        if os.path.isdir(self.files):
            shutil.rmtree(self.files, True)
            os.mkdir(self.files)

    def genops(self):
        '''Sets all features'''
        printc('Setting up configuration of VRouter ' + str(self.name), 'blue')
        self.clean()
        boot = mpath(NemuVar.nemurcd, NemuVar.rcdboot)
        if not os.path.isfile(self.boot):
            shutil.copyfile(boot, self.boot)
        kernel = mpath(NemuVar.nemurcd, NemuVar.rcdrouter, NemuVar.rcdkernel)
        initrd = mpath(NemuVar.nemurcd, NemuVar.rcdrouter, NemuVar.rcdinitrd)
        if not os.path.isfile(self.kernel):
            shutil.copyfile(kernel, self.kernel)
        if not os.path.isfile(self.initrd):
            shutil.copyfile(initrd, self.initrd)
        if not os.path.isfile(self.rfs):
            for i in self.services:
                i.run(self)
            VFs(self.rcd, 'squash', os.path.basename(self.rfs))
        self.clean()
        printok()

    def start(self):
        self.genops()
        VNode.start(self)
    
    def stop(self):
        if self.running():
            VNode.stop(self)

    def __str__(self):
        com = 'VRouter('
        com += 'name=' + printstr(self.name)
        com += ',' + self.estr_hds()
        com += ',' + self.estr_net()
        com += ',services=['
        for i in self.services:
            com += str(i) + ','
        com = com.rstrip(',')
        com += ']'
        com += ',**' + printstr(self.opts)
        com += ')'
        return com
    
    def qstr(self):
        ret = VNode.qstr(self)
	ret += '-kernel ' + str(self.kernel) + ' '
	ret += '-initrd ' + str(self.initrd) + ' '
        ret += '-boot order=d,menu=on,splash=' + str(self.boot) + ',splash-time=2000 '
        ret += '-cdrom ' + str(self.rfs)
        return ret

    def __repr__(self):
        return str(self)

    def delete(self):
        printc('Deleting the VRouter ' + str(self.name) , 'blue')
        self.stop()
        if os.path.isdir(self.rcd):
            shutil.rmtree(self.rcd, True)
        if os.path.isfile(self.rfs):
            os.unlink(self.rfs)
        del VRouter.set[self.name]
        VNode.delete(self)
        printok()


def GenVRouter(*names):
    '''GenVRouter(**names)
    - Def -
        Forces the configuration of several VRouters.
    - Args -
        . names [list of str] --> names of desired VRouters (default: all VRouters)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(VRouter.set.keys())
    for name in hlist:
        try:
            host = VRouter.set[name]
        except KeyError:
            raise NemuError('Cannot find the VRouter ' + str(name))
        if os.path.isfile(host.rfs):
            os.unlink(host.rfs)
        host.genops()

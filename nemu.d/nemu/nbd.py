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

'''This module manages Remote Network Block Devices'''

import os
import shutil

from nemu.msg import (printc, printstr, printok)
from nemu.proc import (NemuProc, popenproc, killprocl)
from nemu.name import NemuNameSpace
from nemu.error import NemuError
from nemu.var import NemuVar
from nemu.path import mpath

class RemoteNBD():
    '''RemoteNBD(name, hd, cli, addr, port)
    - Def -
        Creates a Remote Network Block Device.
    - Args -
        . name [str] --> name of the RemoteNBD
        . hd [str] --> desired VFs
        . cli [int] --> maximum number of clients supported (Default: 1)
        . addr [str] --> the RemoteNBD server real address (Default: local)
        . port [int] --> the RemoteNBD server real port (Default: autoset)
    '''
    set = dict()
    def __init__(self, name, hd=None, cli=1, addr=None, port=None):
        printc('Setting up the Network Block Device ' + str(name), 'blue')
        if name in RemoteNBD.set:
            raise NemuError('Network Block Device configuration ' + str(name) + ' already exists')
        if name in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        self.name = name
        self.addr = addr
        self.port = port
        self.proc = None
        if not isinstance(cli, int) or cli < 1:
            raise NemuError('Network Block Devices must have a max number of clients greater than 0')
        self.cli = cli
        self.hd = hd
        self.vard = mpath(NemuVar.fsd, str(self.name))
        self.logfile = mpath(self.vard, NemuVar.outlog)
        if not os.path.isdir(self.vard):
            os.mkdir(self.vard)
        if self.addr == None:
            self.addr = NemuVar.taddr
        if self.port == None:
            self.port = NemuVar.tport
            while NemuVar.tport in NemuVar.tports:
                NemuVar.tport += 1
                self.port = NemuVar.tport
        if self.port in NemuVar.tports:
            printc(str(self.port) + " is already in used", "magenta")
        if self.port:
            NemuVar.tports.append(self.port)
        RemoteNBD.set[self.name] = self
        NemuNameSpace.set[name] = self
        printok()

    def running(self):
        '''Indicates if the process runs'''
        return not self.proc == None and self.proc.pid in NemuProc.procl

    def start(self):
        '''Starts the process'''
        if self.running() or self.hd == None:
            return
        printc('Launching the Network Block Device ' + str(self.name), 'blue')
        self.proc = popenproc(self.qstr(), logfile=self.logfile)
        printok()

    def stop(self):
        '''Stops the process'''
        if self.running() and not self.hd == None:
            printc('Stopping the Network Block Device ' + str(self.name) , 'cyan')
            killprocl(self.proc.pid)

    def delete(self):
        '''Deletes the NBD'''
        printc('Deleting the Network Block Device ' + str(self.name), 'blue')
        self.stop()
        if os.path.isdir(self.vard):
            shutil.rmtree(self.vard, True)
        del RemoteNBD.set[self.name]
        del NemuNameSpace.set[self.name]
        printok()

    def qstr(self):
        '''QEMU string'''
        com = 'qemu-nbd --persistent --verbose --shared=' + str(self.cli) + ' --port ' + str(self.port) + ' --bind ' + self.addr + ' '
        if self.hd.mode == 'ro':
            com += '--read-only '
        com += self.hd.path
        return com

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ',hd='
        if self.hd:
            com += str(self.hd)
        else:
            com += printstr(self.hd)
        com += ',cli=' + printstr(self.cli)
        com += ',addr=' + printstr(self.addr)
        com += ',port=' + printstr(self.port)
        com += ')'
        return com

    def __repr__(self):
        return str(self)

def StartRemoteNBD(*names):
    '''StartRemoteNBD(*names)
    - Def -
        Starts some Remote Network Block Device.
    - Args -
        . names [list of str] --> names of the desired NBD servers (Default: starts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNBD.set.keys())
    for i in hlist:
        try:
            nbd = RemoteNBD.set[i]
        except KeyError:
            raise NemuError('Cannot find the Network Block Device ' + str(i))
        nbd.start()

def StopRemoteNBD(*names):
    '''StopRemoteNBD(*names)
    - Def -
        Stops some Remote Network Block Device.
    - Args -
        . names [list of str] --> names of the desired NBD servers (Default: stops all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNBD.set.keys())
    for i in hlist:
        try:
            nbd = RemoteNBD.set[i]
        except KeyError:
            raise NemuError('Cannot find the Network Block Device ' + str(i))
        nbd.stop()

def DelRemoteNBD(*names):
    '''DelRemoteNBD(*names)
    - Def -
        Deletes some Remote Network Block Device.
    - Args -
        . names [list of str] --> names of the desired NBD servers (Default: deletes all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNBD.set.keys())
    for i in hlist:
        try:
            nbd = RemoteNBD.set[i]
        except KeyError:
            raise NemuError('Cannot find the Network Block Device ' + str(i))
        nbd.delete()

def nbdvfs(name):
    '''nbdfs(name)
    - Def -
         Returns a format Remote Network Block Device path string for VFs.
    - Args -
        . name [str] --> name of the desired NBD
    '''
    try:
        nbd = RemoteNBD.set[name]
    except KeyError:
        raise NemuError('Cannot find the Network Block Device ' + str(name))
    return 'nbd:' + str(nbd.addr) + ':' + str(nbd.port)

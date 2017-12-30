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

'''This module contains session constants and variables'''

import platform
from nemu.path import mpath

class _NemuVar:
    '''Core NEmu var core data base'''
    nemud = ''
    nemurcd = 'rcd'
    workspace = ''
    session = ''
    rootd = ''
    fsd = 'fs'
    noded = 'node'
    linkd = 'link'
    rcd = 'rcd'
    rcdkernel = 'kernel'
    rcdinitd = 'init.d'
    rcdinitrd = 'initrd.gz'
    rcdtcz = 'tcz'
    rcdfiles = 'files'
    rcdboot = 'nemu.bmp'
    rcdrouter = 'vrouter'
    rcond = 'rcon'
    mobd = 'mob'
    lnf = ''
    logf = 'nemu.log'
    dotf = 'nemu.dot'
    conff = 'nemu.conf'
    histf = 'nemu.hist'
    vde = 'vde'
    vnd = 'vnd'
    nemo = 'nemo'
    qemu = 'qemu-system-x86_64'
    inlog = 'in.log'
    outlog = 'out.log'
    tapup = 'tap-ifup'
    tapdown = 'tap-ifdown'
    hdcopy = True
    color = True
    this = None
    quiet = False
    uaddr = '127.0.0.1'
    taddr = '127.0.0.1'
    maddr = '230.0.0.1'
    uport = 7000
    uports = list()
    tport = 7000
    tports = list()
    mport = 7000
    mports = list()
    hwaddr = 'a2:00:00:00:00:00'
    hwaddrs = list()
    arch = platform.system() + '-' + platform.machine()
    kvm = mpath('/', 'dev', 'kvm')
    rflag = '__;;__'
    def __init__(self):
        NemuVar.nemud = _NemuVar.nemud
        NemuVar.nemurcd = _NemuVar.nemurcd
        NemuVar.workspace = _NemuVar.workspace
        NemuVar.session = _NemuVar.session
        NemuVar.rootd = _NemuVar.rootd
        NemuVar.fsd = _NemuVar.fsd
        NemuVar.noded = _NemuVar.noded
        NemuVar.linkd = _NemuVar.linkd
        NemuVar.rcd = _NemuVar.rcd
        NemuVar.rcdkernel = _NemuVar.rcdkernel
        NemuVar.rcdinitd = _NemuVar.rcdinitd
        NemuVar.rcdinitrd = _NemuVar.rcdinitrd
        NemuVar.rcdtcz = _NemuVar.rcdtcz
        NemuVar.rcdfiles = _NemuVar.rcdfiles
        NemuVar.rcdboot = _NemuVar.rcdboot
        NemuVar.rcdrouter = _NemuVar.rcdrouter
        NemuVar.rcond = _NemuVar.rcond
        NemuVar.mobd = _NemuVar.mobd
        NemuVar.lnf = _NemuVar.lnf
        NemuVar.logf = _NemuVar.logf
        NemuVar.dotf = _NemuVar.dotf
        NemuVar.conff = _NemuVar.conff
        NemuVar.histf = _NemuVar.histf
        NemuVar.vde = _NemuVar.vde
        NemuVar.vnd = _NemuVar.vnd
        NemuVar.nemo = _NemuVar.nemo
        NemuVar.qemu = _NemuVar.qemu
        NemuVar.inlog = _NemuVar.inlog
        NemuVar.outlog = _NemuVar.outlog
        NemuVar.tapup = _NemuVar.tapup
        NemuVar.tapdown = _NemuVar.tapdown
        NemuVar.hdcopy = _NemuVar.hdcopy
        NemuVar.color = _NemuVar.color
        NemuVar.quiet = _NemuVar.quiet
        NemuVar.this = _NemuVar.this
        NemuVar.taddr = _NemuVar.taddr
        NemuVar.tport = _NemuVar.tport
        NemuVar.tports = _NemuVar.tports
        NemuVar.uaddr = _NemuVar.uaddr
        NemuVar.uport = _NemuVar.uport
        NemuVar.uports = _NemuVar.uports
        NemuVar.maddr = _NemuVar.maddr
        NemuVar.mport = _NemuVar.mport
        NemuVar.mports = _NemuVar.mports
        NemuVar.hwaddr = _NemuVar.hwaddr
        NemuVar.hwaddrs = _NemuVar.hwaddrs
        NemuVar.arch = _NemuVar.arch
        NemuVar.kvm = _NemuVar.kvm
        NemuVar.rflag = _NemuVar.rflag

class NemuVar(_NemuVar):
    '''Current NEmu var core data base'''
    def __init__(self):
        _NemuVar.__init__(self)

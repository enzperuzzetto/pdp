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

'''This module manages virtual interfaces'''

from nemu.error import NemuError
from nemu.msg import (printc, printstr)
from nemu.addr import (hws2hwt, hwt2hws)
from nemu.var import NemuVar
from nemu.name import NemuNameSpace

import os

class Iface():
    '''Common Interface core definition'''
    def __init__(self):
        self.addr = None
        self.port = None
        self.proto = None
        self.type = None
        self.laddr = None
        self.lport = None
        self.id = None
        self.dump = False
        self.dumpfile = None
        self.hostname = None
        self.net = None
        self.restrict = False
        self.forwards = None
        self.activ = False

    def set(self, addr, port, type, proto, laddr, lport):
        '''Sets connection information'''
        self.type = type
        self.proto = proto
        if not addr:
            if self.proto == 'tcp':
                self.addr = NemuVar.taddr
            elif self.proto == 'udp':
                self.addr = NemuVar.uaddr
            elif self.proto == 'mcast':
                self.addr = NemuVar.maddr
        else:
            self.addr = addr
        if not port:
            if self.proto == 'tcp':
                self.port = NemuVar.tport
                while NemuVar.tport in NemuVar.tports:
                    NemuVar.tport += 1
                    self.port = NemuVar.tport
            if self.proto == 'udp':
                self.port = NemuVar.uport
                while NemuVar.uport in NemuVar.uports:
                    NemuVar.uport += 1
                    self.port = NemuVar.uport
            if self.proto == 'mcast':
                self.port = NemuVar.mport
                while NemuVar.mport in NemuVar.mports:
                    NemuVar.mport += 1
                    self.port = NemuVar.mport
        else:
            self.port = port
        if self.proto == 'tcp' and self.type == 'srv' and self.port in NemuVar.tports:
            printc(str(self.port) + " is already in used", "magenta")
        if self.proto == 'udp' and self.type == 'srv' and self.port in NemuVar.uports:
            printc(str(self.port) + " is already in used", "magenta")
        if self.proto == 'udp' and self.type == 'peer' and self.port in NemuVar.uports:
            printc(str(self.port) + " is already in used", "magenta")
        if self.proto == 'mcast' and self.type == 'srv' and self.port in NemuVar.mports:
            printc(str(self.port) + " is already in used", "magenta")
        if self.proto == 'tcp' and self.port:
            NemuVar.tports.append(self.port)
        if self.proto == 'udp' and self.port:
            NemuVar.uports.append(self.port)
        if self.proto == 'mcast' and self.port:
            NemuVar.mports.append(self.port)
        if proto == 'udp':
            if self.laddr == None:
                self.laddr = NemuVar.uaddr
            else:
                self.laddr = laddr
            if not lport:
                self.lport = NemuVar.uport
                while NemuVar.uport in NemuVar.uports:
                    NemuVar.uport += 1
                    self.lport = NemuVar.uport
            else:
                self.lport = lport
        if self.lport in NemuVar.uports and self.type == 'srv':
            printc(str(self.lport) + " is already in used", "magenta")
        if self.lport in NemuVar.uports and self.type == 'peer':
            printc(str(self.lport) + " is already in used", "magenta")
        if self.lport:
            NemuVar.uports.append(self.lport)
        self.activ = True
        
    def unset(self):
        '''Unsets connection information'''
        if self.proto == 'tcp' and self.port in NemuVar.tports:
            NemuVar.tports.remove(self.port)
        if self.proto == 'udp' and self.port in NemuVar.uports:
            NemuVar.uports.remove(self.port)
        if self.proto == 'mcast' and self.port in NemuVar.mports:
            NemuVar.mports.remove(self.port)
        if self.lport in NemuVar.uports:
            NemuVar.uports.remove(self.lport)
        self.addr = None
        self.port = None
        self.laddr = None
        self.lport = None
        self.type = None
        self.proto = None
        self.activ = False
        self.net = None
        self.restrict = False
        self.forwards = None

    def delete(self):
        '''Deletes the iface'''
        self.unset()
        if self.dumpfile and os.path.isfile(self.dumpfile):
            os.unlink(self.dumpfile)

    def slirp(self, net, restrict, forwards, type):
        '''Sets slirp information'''
        self.proto = 'slirp'
        self.type = type
        self.net = net
        self.restrict = restrict
        self.forwards = forwards
        self.activ = True

    def unix(self, path, type):
        '''Sets unix information'''
        self.proto = 'unix'
        self.type = type
        self.addr = path
        self.activ = True

    def vde(self, path, type):
        '''Sets vde information'''
        self.proto = 'vde'
        self.type = type
        self.addr = path
        self.activ = True
        
    def tap(self, ifname, type):
        '''Sets tap information'''
        self.proto = 'tap'
        self.type = type
        self.addr = ifname
        self.activ = True

    def __str__(self):
        com = ''
        if self.activ and self.type == 'srv':
            if self.proto == 'slirp':
                com += 'SetIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',proto=' +  printstr(self.proto) + ')'
            elif self.proto == 'vde':
                com += 'SetIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',proto=' +  printstr(self.proto) + ',addr=' + printstr(self.addr) + ')'
            elif self.proto == 'unix':
                com += 'SetIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',proto=' +  printstr(self.proto) + ',addr=' + printstr(self.addr) + ')'
            elif self.proto == 'tap':
                com += 'SetIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',proto=' +  printstr(self.proto) + ',addr=' + printstr(self.addr) + ')'
            else:
                com += 'SetIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',addr=' + printstr(self.addr)
                com += ',port=' + printstr(self.port) + ',proto=' +  printstr(self.proto)
                com += ',laddr=' + printstr(self.laddr) + ',lport=' + printstr(self.lport) + ')'
        elif self.activ and self.type == 'peer':
            com += 'SetAirIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',addr=' + printstr(self.addr)
            com += ',port=' + printstr(self.port)
            com += ',laddr=' + printstr(self.laddr) + ',lport=' + printstr(self.lport) + ')'
        #if self.activ and self.dump:
        #    if len(com) > 0 :
        #        com += "\n"
        #    com += 'DumpIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ')'
        return com

    def __repr__(self):
        return self.__class__.__name__ + '()'

class VNic(Iface):
    '''VNic(hw, model)
    - Def -
        Creates a VNode network interface card.
    - Args -
        . hw [str] --> "x:x:x:x:x:x": host interface mac address (Default: autoset)
        . model [str] --> model of network interface card (Default: e1000)
    '''
    def __init__(self, hw=None, model='e1000'):
        Iface.__init__(self)
        self.hw = hw
        self.model = model
        
        if not self.hw:
            hwt = hws2hwt(NemuVar.hwaddr)
            if hwt[5] == 254:
                if hwt[4] == 254:
                    if hwt[3] == 254:
                        if hwt[2] == 254:
                            raise NemuError('Too many Interfaces')
                        else:
                            hwt[2] += 1
                            hwt[3] = 0
                            hwt[4] = 0
                            hwt[5] = 1
                    else:
                        hwt[3] += 1
                        hwt[4] = 0
                        hwt[5] = 1
                else:
                    hwt[4] += 1
                    hwt[5] = 1
            else:
                hwt[5] += 1
            hws = hwt2hws(hwt)
            NemuVar.hwaddr = hws
            self.hw = hws
        if self.hw in NemuVar.hwaddrs:
            printc(str(self.hw) + " is already in used", "magenta")
        NemuVar.hwaddrs.append(self.hw)
                
    def delete(self):
        Iface.delete(self)
        if self.hw in NemuVar.hwaddrs:
            NemuVar.hwaddrs.remove(self.hw)

    def __str__(self):
        return Iface.__str__(self)

    def __repr__(self):
        com = self.__class__.__name__ + '('
        com += 'hw=' + printstr(self.hw)
        com += ',model=' + printstr(self.model)
        com += ')'
        return com


class VLic(Iface):
    '''VLic(id)
    - Def -
        Creates a VLink network interface card.
    - Args -
        . id [str] --> the interface id
    '''
    def __init__(self, id):
        Iface.__init__(self)
        if not id:
            raise NemuError('A VLic needs an id')
        self.id = id
        self.instate = None
        self.invli = None
        self.inbw = None
        self.indl = None
        self.injit = None
        self.inber = None
        self.inqs = None
        self.outstate = None
        self.outvli = None
        self.outbw = None
        self.outdl = None
        self.outjit = None
        self.outber = None
        self.outqs = None

    def conf(self, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Sets iface properties'''
        if direction == "in" or direction == "all":
            if not state == None:
                self.instate = state
            if not vlan == None:
                self.invli = vlan
            if not bandwidth == None:
                self.inbw = bandwidth
            if not delay == None:
                self.indl = delay
            if not jitter == None:
                self.injit = jitter
            if not ber == None:
                self.inber = ber
            if not qsize == None:
                self.inqs = qsize
        if direction == "out" or direction == "all":
            if not state == None:
                self.outstate = state
            if not vlan == None:
                self.outvli = vlan
            if not bandwidth == None:
                self.outbw = bandwidth
            if not delay == None:
                self.outdl = delay
            if not jitter == None:
                self.outjit = jitter
            if not ber == None:
                self.outber = ber
            if not qsize == None:
                self.outqs = qsize

    def unconf(self, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Unsets iface properties'''
        if direction == "in" or direction == "all":
            if state:
                self.instate = None
            if vlan:
                self.invli = None
            if bandwidth:
                self.inbw = None
            if delay:
                self.indl = None
            if jitter:
                self.injit = None
            if ber:
                self.inber = None
            if qsize:
                self.inqs = None
        if direction == "out" or direction == "all":
            if state:
                self.outstate = None
            if vlan:
                self.outvli = None
            if bandwidth:
                self.outbw = None
            if delay:
                self.outdl = None
            if jitter:
                self.outjit = None
            if ber:
                self.outber = None
            if qsize:
                self.outqs = None

    def __str__(self):
        com = Iface.__str__(self)
        if self.instate or self.invli or self.inbw or self.indl or self.injit or self.inber or self.inqs:
            if len(com) > 0 :
                com += "\n"
            com += 'ConfIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',direction="in"'
            com += ',state=' + printstr(self.instate) + ',vlan=' + printstr(self.invli) + ',bandwidth=' + printstr(self.inbw)
            com += ',delay=' + printstr(self.indl) + ',jitter=' + printstr(self.injit) + ',ber=' + printstr(self.inber) 
            com += ',qsize=' + printstr(self.inqs) + ')'
        if self.outstate or self.outvli or self.outbw or self.outdl or self.outjit or self.outber or self.outqs:
            if len(com) > 0 :
                com += "\n"
            com += 'ConfIface(name=' + printstr(self.hostname + ':' + str(self.id)) + ',direction="out"'
            com += ',state=' + printstr(self.outstate) + ',vlan=' + printstr(self.outvli) + ',bandwidth=' + printstr(self.outbw)
            com += ',delay=' + printstr(self.outdl) + ',jitter=' + printstr(self.outjit) + ',ber=' + printstr(self.outber) 
            com += ',qsize=' + printstr(self.outqs) + ')'
        return com

    def __repr__(self):
        com = self.__class__.__name__ + '('
        com += 'id=' + printstr(self.id)
        com += ')'
        return com

def DumpIface(*names):
    '''DumpIface(*names)
    - Def -
        Dumps the traffic of several virtual interfaces.
    - Args -
        . names [list of str] --> the <VNode name>[:<interface>] or <VLink name>[:<interface>] (Default: all interfaces)
    '''
    for name in names:
        hostname = name
        iface = None
        vobj = None
        if ':' in name:
            iface = str(name.split(':')[1])
            hostname = str(name.split(':')[0])
        try:
            vobj = NemuNameSpace.set[hostname]
        except KeyError:
            raise NemuError('Cannot find ' + str(hostname))
        if iface == None:
            for iface in vobj.ifaces:
                DumpIface(hostname + ':' + str(iface.id))
        else:
            vobj.dumpiface(iface)

def UndumpIface(*names):
    '''UndumpIface(*names)
    - Def -
        Stops to dump the the traffic of several interfaces.
    - Args -
        . names [list of str] --> the <VNode name>[:<interface>] or <VLink name>[:<interface>] (Default: all interfaces)
    '''
    for name in names:
        hostname = name
        iface = None
        vobj = None
        if ':' in name:
            iface = str(name.split(':')[1])
            hostname = str(name.split(':')[0])
        try:
            vobj = NemuNameSpace.set[hostname]
        except KeyError:
            raise NemuError('Cannot find ' + str(hostname))
        if iface == None:
            for iface in vobj.ifaces:
                UndumpIface(hostname + ':' + str(iface.id))
        else:
            vobj.undumpiface(iface)

def AddIface(name, iface=None):
    '''AddIface(name, iface)
    - Def -
        Creates a new virtual interface.
    - Args -
        . name [str] --> the <VLink|VNode name>
        . iface [str|VNic] --> new id for VLink and VNic for VNode (Default: autoset)
    '''
    iface = iface
    hostname = name
    vobj = None
    try:
        vobj = NemuNameSpace.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(hostname))
    vobj.addiface(iface)

def SetIface(name, addr=None, port=None, proto=None, laddr=None, lport=None):
    '''SetIface(name, addr, port, proto, laddr, lport)
    - Def -
        Setup configuration of a virtual interface.
    - Args -
        . name [str] --> the <VLink|VNode name>[:<interface>] (Default: first interface)
        . addr [str] --> the VLink|VNode interface real address (Default: local)
        . port [int] --> the VLink|VNode interface real port (Default: autoset)
        . proto [str] --> the VLink|VNode interface real proto "tcp", "udp", "vde", "tap", "slirp" (Default: autoset)
        . laddr [str] --> the VLink|VNode interface real udp local address (Default: local)
        . lport [int] --> the VLink|VNode interface real udp local port (Default: autoset)
    '''
    iface = None
    hostname = name
    vobj = None
    if ':' in name:
        iface = str(name.split(':')[1])
        hostname = str(name.split(':')[0])
    try:
        vobj = NemuNameSpace.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(hostname))
    if iface == None:
        for iface in vobj.ifaces:
            SetIface(hostname + ':' + str(iface.id), addr, port, proto, laddr, lport)
            break
    else:
        vobj.setiface(iface, addr, port, 'srv', proto, laddr, lport)

def UnsetIface(name):
    '''UnsetIface(name)
    - Def -
        Unset the connection setup of virtual interfaces.
    - Args -
        . name [str] --> the <VLink|VNode name>[:<interface>] (Default: all interfaces)
    '''
    hostname = name
    iface = None
    vobj = None
    if ':' in name:
        iface = str(name.split(':')[1])
        hostname = str(name.split(':')[0])
    try:
        vobj = NemuNameSpace.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(name))
    if iface == None:
        for iface in vobj.ifaces:
            UnsetIface(str(hostname) + ':' + str(iface.id))
    else:
        vobj.unsetiface(iface)


def ConfIface(name, direction="all", state=None, vlan=None, bandwidth=None, delay=None, jitter=None, ber=None, qsize=None):
    '''ConfIface(name, direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
    - Def -
        Sets properties of several virtual interfaces (only for VLine, VHub, VSwitch, VAirWic, VAirAp).
    - Args -
        . name [str] --> the <VLink name>[:<interface>] (Default: all interfaces)
        . direction [str] --> "all", "in" or "out" (Default: all)
        . state [str] --> "up" or "down" (Default: None)
        . vlan [int] --> port VLAN (Default: None)
        . bandwidth [int] --> bandwidth in bps (Default: None)
        . delay [double] --> delay in seconds (Default: None)
        . jitter [double] --> delay jitter proportion (Default: None)
        . ber [double] --> bit error rate (Default: None)
        . qsize [int] --> packet-queue length (Default: None)
    '''
    hostname = name
    iface = None
    vobj = None
    if ':' in name:
        iface = str(name.split(':')[1])
        hostname = str(name.split(':')[0])
    try:
        vobj = NemuNameSpace.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(hostname))
    if iface == None:
        for iface in vobj.ifaces:
            ConfIface(hostname + ':' + str(iface.id), direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
    else:
        vobj.confiface(iface, direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
        

def UnconfIface(name, direction="all", state=True, vlan=True, bandwidth=True, delay=True, jitter=True, ber=True, qsize=True):
    '''UnconfIface(name, direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
    - Def -
        Unsets properties of several virtual interfaces.
    - Args -
        . name [str] --> the <VLink name>[:<interface>] (Default: all interfaces)
        . direction [str] --> "all", "in" or "out" (Default: all)
        . state [bool] --> (Default: True)
        . vlan [bool] --> port VLAN (Default: True)
        . bandwidth [bool] --> bandwidth in bps (Default: True)
        . delay [bool] --> delay in seconds (Default: True)
        . jitter [bool] --> delay jitter proportion (Default: True)
        . ber [bool] --> bit error rate (Default: True)
        . qsize [bool] --> packet-queue length (Default: True)
    '''
    hostname = name
    iface = None
    vobj = None
    if ':' in name:
        iface = str(name.split(':')[1])
        hostname = str(name.split(':')[0])
    try:
        vobj = NemuNameSpace.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(name))
    if iface == None:
        for iface in vobj.ifaces:
            UnconfIface(str(hostname) + ':' + str(iface.id), direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
    else:
        vobj.unconfiface(iface, direction, state, vlan, bandwidth, delay, jitter, ber, qsize)

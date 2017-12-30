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

'''This module manages virtual links VQHub'''

from nemu.msg import (printc, printok, printstr)
from nemu.vlink import VLink
from nemu.error import NemuError
from nemu.var import NemuVar

class VQHub(VLink):
    '''VQHub(name, niface)
    - Def -
        A VQHub represents a virtual Ethernet QEMU hub.
    - Args -
        . name [str] --> VLink name
        . niface [str] --> number of virtual interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VQHub ' + str(name), 'blue')
        VLink.__init__(self, name, "qhub", niface)
        VQHub.set[name] = self
        self.qemu = NemuVar.qemu
        printok()

    def delete(self):
        printc('Deleting the VQHub ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VQHub.set[self.name]
        printok()

    def addlink(self, id):
        return

    def dellink(self, id):
        return
    
    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VQHub")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VQHub")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VQHub")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VQHub")

    def unixiface(self, id, path, type):
        raise NemuError('UNIX is not available for VQHub')

    def setlink(self, id):
        iface = self.findiface(id)
        if self.running() and iface.activ:
            self.unsetlink(id)
            com = "host_net_add socket " + "name=socket." + str(iface.id) + ",vlan=" + str(0) + ","
            if iface.proto == 'mcast':
                com += "mcast" + "=" + str(iface.addr) + ":" + str(iface.port)
                if iface.laddr:
                    com += ",localaddr="  + str(iface.laddr)
            elif iface.proto == 'udp':
                if iface.type == 'srv':
                    com += "udp" + "=" + str(iface.laddr) + ":" + str(iface.lport) + ",localaddr=" + str(iface.addr) + ':' +  str(iface.port)
                elif iface.type == 'cli':
                    com += "udp" + "=" + str(iface.addr) + ":" + str(iface.port) + ",localaddr=" + str(iface.laddr) + ':' +  str(iface.lport)
            elif iface.proto == 'tcp': 
                proto = None
                if iface.type == 'srv':
                    proto = 'listen'
                elif iface.type == 'cli':
                    proto = 'connect'
                com += str(proto) + "=" + str(iface.addr) + ":" + str(iface.port)
            elif iface.proto == 'vde':
                com = "host_net_add vde name=socket." + str(iface.id) + ",vlan=" + str(iface.id) + ",sock=" + str(iface.path)
            elif iface.proto == 'tap':
                com = "host_net_add tap name=socket." + str(iface.id) + ",vlan=" + str(iface.id) + ",ifname=" + str(iface.ifname) + ",script=" + str(NemuVar.tapup) + ",downscript=" + str(NemuVar.tapdown)
            self.send(com)

    def unsetlink(self, id):
        iface = self.findiface(id)
        if self.running():
            self.send("host_net_remove 0 socket." + str(iface.id))

    def qstr(self):
        return str(self.qemu) + ' -name ' + str(self.name)  + ' -monitor stdio -m 2 -localtime -vnc none -net nic,vlan=0'
    
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',niface=' + printstr(len(self.ifaces)) + ')'
        com += VLink.__str__(self)
        return com
    
    def __repr__(self):
        return str(self)

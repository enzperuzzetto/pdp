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

'''This module manages virtual links TAP'''

from nemu.msg import (printc, printok, printstr)
from nemu.vlink import VLink
from nemu.proc import (popenproc, NemuProc, killprocl)
from nemu.error import NemuError

class VTap(VLink):
    '''VTap(name, iface)
    - Def -
        A VTap represents a virtual connection to a Tap interface laying on the physical host.
    - Args -
        . name [str] --> VTap name
        . iface [str] --> Tap interface
    '''
    set = dict()
    def __init__(self, name, iface):
        printc('Setting up VTap ' + str(name), 'blue')
        VLink.__init__(self, name, "tap", 1)
        self.ifname = iface
        self.activ = False
        VTap.set[name] = self
        printok()

    def addlink(self, id):
        return

    def dellink(self, id):
        return

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VTap")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VTap")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VTap")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VTap")

    def setiface(self, id, addr, port, type, proto, laddr, lport):
        VLink.tapiface(self, id, ifname=self.ifname, type='srv')
        
    def setlink(self, id):
        return

    def unsetlink(self, id):
        return

    def start(self):
        self.activ = True
        return

    def stop(self):
        self.activ = False
        return

    def reboot(self):
        return

    def send(self, line):
        return

    def delete(self):
        printc('Deleting the VTap ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VTap.set[self.name]
        printok()

    def running(self):
        return self.activ
    
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) 
        com += ',iface=' + printstr(self.ifname) + ')'
        return com

    def __repr__(self):
        return str(self)

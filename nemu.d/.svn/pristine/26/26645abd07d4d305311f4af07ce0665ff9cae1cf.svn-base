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

'''This module manages virtual links VDE'''

from nemu.var import NemuVar
from nemu.msg import (printc, printok, printstr)
from nemu.path import mpath
from nemu.vlink import VLink
from nemu.error import NemuError

import shutil
import os


class VDEHub(VLink):
    '''VDEHub(name, niface)
    - Def -
        A VDEHub represents a Virtual Distributed Ethernet hub.
    - Args -
        . name [str] --> VDEHub name
        . niface [str] --> number of virtual interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VDEHub ' + str(name), 'blue')
        VLink.__init__(self, name, "vde", niface)
        VDEHub.set[name] = self
        self.vdepath = mpath(self.vard, NemuVar.vde)
        printok()

    def delete(self):
        printc('Deleting the VDEHub ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VDEHub.set[self.name]
        printok()

    def addlink(self, id):
        if self.running():
            self.send("port/setnumports " + str(len(self.ifaces)))

    def dellink(self, id):
        return

    def setiface(self, id, addr, port, type, proto, laddr, lport):
        VLink.vdeiface(self, id, path=self.vdepath, type='srv')

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VDE")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VDE")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VDE")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VDE")

    def setlink(self, id):
        return

    def unsetlink(self, id):
        return

    def stop(self):
        if self.running():
            VLink.stop(self)
            if os.path.exists(self.vdepath):
                shutil.rmtree(self.vdepath)

    def qstr(self):
        return 'vde_switch -F --hub -n ' + str(len(self.ifaces)+1) + ' -s ' + self.vdepath
    
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',niface=' + printstr(len(self.ifaces)) + ')'
        return com

    def __repr__(self):
        return str(self)
    

class VDESwitch(VLink):
    '''VDESwitch(name, niface)
    - Def -
        A VDESwitch represents a Virtual Distributed Ethernet switch.
    - Args -
        . name [str] --> VDESwitch name
        . niface [str] --> number of virtual interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VDESwitch ' + str(name), 'blue')
        VLink.__init__(self, name, "vde", niface)
        VDESwitch.set[name] = self
        self.vdepath = mpath(self.vard, NemuVar.vde)
        printok()

    def delete(self):
        printc('Deleting the VDESwitch ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VDESwitch.set[self.name]
        printok()

    def addlink(self, id):
        if self.running():
            self.send("port/setnumports " + str(len(self.ifaces)))

    def dellink(self, id):
        return

    def setiface(self, id, addr, port, type, proto, laddr, lport):
        VLink.vdeiface(self, id, path=self.vdepath, type='srv')

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VDE")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VDE")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VDE")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VDE")

    def setlink(self, id):
        return

    def unsetlink(self, id):
        return

    def stop(self):
        if self.running():
            VLink.stop(self)
            if os.path.exists(self.vdepath):
                shutil.rmtree(self.vdepath)

    def qstr(self):
        return 'vde_switch -F -n ' + str(len(self.ifaces)+1) + ' -s ' + self.vdepath
    
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',niface=' + printstr(len(self.ifaces)) + ')'
        return com

    def __repr__(self):
        return str(self)

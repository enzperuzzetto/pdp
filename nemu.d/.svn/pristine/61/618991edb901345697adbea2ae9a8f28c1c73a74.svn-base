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

'''This module manages virtual links VND'''

from nemu.msg import (printc, printok, printstr)
from nemu.vlink import VLink


class VLine(VLink):
    '''VLine(name)
    - Def -
        A VLine represents a virtual Ethernet cable.
    - Args -
        . name [str] --> VLine name
    '''
    set = dict()
    def __init__(self, name):
        printc('Setting up VLine ' + str(name), 'blue')
        VLink.__init__(self, name, "line", 2)
        VLine.set[name] = self
        printok()

    def addiface(self, id=None):
        return

    def deliface(self, id):
        return

    def setvlink(self):
        self.send("mode link")
        self.send("tie i0 i1")
        self.send("tie i1 i0")
        
    def unsetvlink(self):
        self.send("untie i0 i1")
        self.send("untie i1 i0")

    def delete(self):
        printc('Deleting the VLine ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VLine.set[self.name]
        printok()
        
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ')'
        return com

    def __repr__(self):
        return str(self)
    

class VHub(VLink):
    '''VHub(name, niface)
    - Def -
        A VHub represents a virtual Ethernet hub.
    - Args -
        . name [str] --> VHub name
        . niface [int] --> number of virtual interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VHub ' + str(name), 'blue')
        VLink.__init__(self, name, "hub", niface)
        VHub.set[name] = self
        printok()

    def setvlink(self):
        self.send("mode hub")

    def delete(self):
        printc('Deleting the VHub ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VHub.set[self.name]
        printok()

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',niface=' + printstr(len(self.ifaces)) + ')'
        return com

    def __repr__(self):
        return str(self)


class VSwitch(VLink):
    '''VSwitch(name, niface)
    - Def -
        A VSwitch represents a virtual Ethernet switch.
    - Args -
        . name [str] --> VSwitch name
        . niface [int] --> number of virtual interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VSwitch ' + str(name), 'blue')
        VLink.__init__(self, name, "switch", niface)
        VSwitch.set[name] = self
        printok()
    
    def setvlink(self):
        self.send("mode switch")

    def delete(self):
        printc('Deleting the VSwitch ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VSwitch.set[self.name]
        printok()

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',niface=' + printstr(len(self.ifaces)) + ')'
        return com

    def __repr__(self):
        return str(self)

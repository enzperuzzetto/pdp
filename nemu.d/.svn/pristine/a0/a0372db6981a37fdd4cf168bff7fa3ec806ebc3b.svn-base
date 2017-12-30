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

'''This module manages virtual links SLIRP'''

from nemu.msg import (printc, printok, printstr)
from nemu.vlink import VLink
from nemu.error import NemuError


class VSlirp(VLink):
    '''VSlirp(name, net, restrict, forwards)
    - Def -
        A VSlirp represents a virtual connection to the real network through a NATed system.
    - Args -
        . name [str] --> VSlirp name
        . net [str] --> network IP with subnet mask (Default: 192.168.0.0/24)
        . restrict [bool] --> isolates the virtual node form the real world
        . forwards [list of Forward] --> forward ports (Default: empty)
    '''
    set = dict()
    def __init__(self, name, net='192.168.0.0/24', restrict=False, forwards=list()):
        printc('Setting up VSlirp ' + str(name), 'blue')
        VLink.__init__(self, name, "slirp", 1)
        VSlirp.set[name] = self
        self.net = net
        self.restrict = restrict
        self.forwards = list()
        for fw in forwards:
            self.forwards.append(fw)
        self.activ = False
        printok()

    def addlink(self, id):
        return

    def dellink(self, id):
        return

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VSlirp")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VSlirp")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VSlirp")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VSlirp")

    def setiface(self, id, addr, port, type, proto, laddr, lport):
        self.slirpiface(id, net=self.net, restrict=self.restrict, forwards=self.forwards, type='srv')

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
        printc('Deleting the VSlirp ' + str(self.name) , 'blue')
        VLink.delete(self)
        del VSlirp.set[self.name]
        printok()

    def running(self):
        return self.activ
    
    def fw_estr(self):
        '''Forward format string'''
        com = 'forwards=['
        for fw in self.forwards:
            if fw == None:
                continue
            com += str(fw)
            if not self.forwards.index(fw) == (len(self.forwards)-1):
                com += ','
        com += ']'
        return com

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ',net=' + printstr(self.net) + ',restrict=' + printstr(self.restrict) + ',' + self.fw_estr() + ')'
        return com

    def __repr__(self):
        return str(self)

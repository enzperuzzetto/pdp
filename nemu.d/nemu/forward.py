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

'''This module manages port forwarding for VSlirp'''

from nemu.msg import (printc, printstr)
from nemu.var import NemuVar

class Forward():
    '''Forward(host, guest, proto)
    - Def -
       Forwards port from the real system to a virtual slirp.
    - Args -
       . host [str] --> [ip]:<port>
       . guest [str] --> [ip]:<port>
       . proto [str] --> tcp or udp (Default: tcp)
        '''
    def __init__(self, host, guest, proto='tcp'):
        self.haddr = NemuVar.uaddr
        self.gaddr = ""
        self.hport = None
        self.gport = None
        self.proto = proto
        if ':' in str(host):
            self.hport = int(host.split(':')[1])
            self.haddr = str(host.split(':')[0])
        else:
            self.hport = int(host)
        if ':' in str(guest):
            self.gport = int(guest.split(':')[1])
            self.gaddr = str(guest.split(':')[0])
        else:
            self.gport = int(guest)
        if self.proto == 'tcp' and self.hport in NemuVar.tports:
            printc(str(self.hport) + " is already in used", "magenta")
        else:
            NemuVar.tports.append(self.hport)
        if self.proto == 'udp' and self.hport in NemuVar.uports:
            printc(str(self.hport) + " is already in used", "magenta")
        else:
            NemuVar.uports.append(self.hport)
              
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'host=' + printstr(self.haddr + ':' + str(self.hport))
        com += ',guest=' + printstr(self.gaddr + ':' + str(self.gport))
        com += ',proto=' + printstr(self.proto)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


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

'''This module manages wireless elements'''

from nemu.error import NemuError
from nemu.msg import (printc, printstr, printok)
from nemu.vlink import VLink

class VAirLink(VLink):
    '''VAirLink(name)
    - Def -
        A VAirLink represents a virtual wireless inter-connection device.
    - Args -
        . name [str] --> VAirLink name
        . type [str] --> VAirLink type
        . niface [int] --> number of NIC
    '''
    set = dict()
    def __init__(self, name, type=None, niface=0):
        VLink.__init__(self, name, type, niface)
        VAirLink.set[name] = self

    def setmode(self, mode):
        '''Sets the Air mode (adhoc or infra)'''
        return

    def gstr(self):
        com = str(self.name) + ' [shape=ellipse,peripheries=3,color='
        if self.running():
            com += 'green'
        else:
            com += 'lightgray'
        com += ',style=filled,label="' + str(self.__class__.__name__) + '['+ str(self.name) + ']"];'
        return com

    def delete(self):
        VLink.delete(self)
        del VAirLink.set[self.name]

class VAirWic(VAirLink):
    '''VAirWic(name)
    - Def -
        A VAirWic represents a virtual wireless network interface.
    - Args -
        . name [str] --> VAirWic name
    '''
    set = dict()
    def __init__(self, name):
        printc('Setting up VAirWic ' + str(name), 'blue')
        VAirLink.__init__(self, name, "wic", 1)
        VAirWic.set[name] = self
        self.wmode = 'adhoc'
        printok()

    def setvlink(self):
        self.send("mode mob-" + str(self.wmode))

    def addlink(self, id):
        if self.running():
            if id == '0':
                self.send("add i" + str(id) + " wic")
            else:
                self.send("add i" + str(id) + " ral")

    def setmode(self, mode):
        self.wmode = mode
        if self.running():
            self.setvlink()

    def delete(self):
        printc('Deleting the VAirWic ' + str(self.name) , 'blue')
        VAirLink.delete(self)
        del VAirWic.set[self.name]
        printok()

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name) + ')'
        com += "\n"
        com += 'SetAirMode('+ printstr(self.name)
        com +=',mode=' + printstr(self.wmode) + ')'
        return com
    
    def __repr__(self):
        return str(self)


class VAirAp(VAirLink):
    '''VAirAp(name)
    - Def -
        A VAirAp represents a virtual wireless access point.
    - Args -
        . name [str] --> VAirAp name
        . niface [int] --> number of interfaces
    '''
    set = dict()
    def __init__(self, name, niface):
        printc('Setting up VAirAp ' + str(name), 'blue')
        VAirLink.__init__(self, name, "wap", niface)
        VAirAp.set[name] = self
        printok()

    def setvlink(self):
        self.send("mode access-point")

    def addiface(self, id=None):
        if not id:
            id = self._get_niface_wrapper()
        VAirLink.addiface(self, id)

    def addlink(self, id):
        if self.running():
            if str(id).isdigit():
                self.send("add i" + str(id) + " nic")
                #self.send("add i" + str(id) + " wic")
            else:
                #self.send("add i" + str(id) + " wic")
                self.send("add i" + str(id) + " ral")

    def delete(self):
        printc('Deleting the VAirAp ' + str(self.name) , 'blue')
        VAirLink.delete(self)
        del VAirAp.set[self.name]
        printok()

    def _get_niface_wrapper(self):
        '''Returns the number of NIC'''
        _map = map(lambda x: True if str(x.id).isdigit() else False, self.ifaces)
        ret = _map.count(True)
        return ret

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ',niface=' + printstr(self._get_niface_wrapper())
        com += ')'
        return com
    
    def __repr__(self):
        return str(self)

class VAirRemote(VAirLink):
    '''VAirRemote(name)
    - Def -
        A VAirRemote represents a remote virtual wireless device.
    - Args -
        . name [str] --> VAirRemote name
    '''
    set = dict()
    def __init__(self, name):
        printc('Setting up VAirRemote ' + str(name), 'blue')
        VAirLink.__init__(self, name, "wremote", 0)
        VAirRemote.set[name] = self
        self.activ = False
        printok()

    def addlink(self, id):
        return

    def dellink(self, id):
        return

    def unixiface(self, id, path, type):
        raise NemuError("UNIX is not available for VAirRemote")

    def vdeiface(self, id, path, type):
        raise NemuError("VDE is not available for VAirRemote")

    def tapiface(self, id, ifname, type):
        raise NemuError("TAP is not available for VAirRemote")

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VAirRemote")

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        raise NemuError("Interface properties are not available for VAirRemote")

    def dumpiface(self, id):
        raise NemuError("Dumping is not available for VAirRemote")

    def undumpiface(self, id):
        raise NemuError("Dumping is not available for VAirRemote")

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

    def running(self):
        return self.activ

    def delete(self):
        printc('Deleting the VAirRemote ' + str(self.name) , 'blue')
        VAirLink.delete(self)
        del VAirRemote.set[self.name]
        printok()

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ')'
        return com
    
    def __repr__(self):
        return str(self)

def SetAirMode(name, mode):
    '''SetAirMode(name, mode)
    - Def -
        Sets VAirWic wireless mode.
    - Args -
        . name [str] --> <VAirLink name>
        . mode [str] --> adhoc or infra
    '''
    vobj = None
    try:
        vobj = VAirWic.set[name]
    except KeyError:
        raise NemuError('Cannot find ' + str(name))
    vobj.setmode(mode)
    printc(name + " is now in " + str(mode) + " mode", "cyan")

def SetAirIface(name, addr=None, port=None, laddr=None, lport=None):
    '''SetAirIface(name, addr, port, laddr, lport)
    - Def -
        Creates a new VAirLink wireless interface configuration.
    - Args -
        . name [str] --> the <VAirLink name>[:<interface>] (Default: 0)
        . addr [str] --> the  VAirLink interface real address (Default: local)
        . port [int] --> the VAirLink interface real port (Default: autoset)
        . laddr [str] --> the VAirLink interface real udp local address (Default: local)
        . lport [int] --> the VAirLink interface real udp local port (Default: autoset)
    '''
    iface = str(0)
    hostname = name
    vobj = None
    if ':' in name:
        iface = str(name.split(':')[1])
        hostname = str(name.split(':')[0])
    try:
        vobj = VAirLink.set[hostname]
    except KeyError:
        raise NemuError('Cannot find ' + str(hostname))
    vobj.addiface(iface)
    vobj.setiface(iface, addr=addr, port=port, type='peer', proto='udp', laddr=laddr, lport=lport)



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
from nemu.vair import VAirLink

class Join():
    '''Join(client, core)
    - Def -
        Join enables a connection between an air virtual interface and another one.
    - Args -
        . client [str] --> <VAirLink name>
        . core [str] --> <VAirLink name>
    '''
    set = dict()
    def __init__(self, client, core):
        corename = str(core)
        clientname = str(client)
        try:
            self.core = VAirLink.set[corename]
        except KeyError:
            raise NemuError('Cannot find ' + str(corename))
        try:
            self.client = VAirLink.set[clientname]
        except KeyError:
            raise NemuError('Cannot find ' + str(clientname))
        self.coreiface = None
        self.clientiface = None
        if not self.core.isiface(clientname):
            self.core.addiface(clientname)
            self.core.setiface(clientname, addr=None, port=None, type='peer', proto='udp', laddr=None, lport=None)
        self.coreiface = self.core.findiface(clientname)
        if not self.client.isiface(corename):
            self.client.addiface(corename)
            self.client.setiface(corename, addr=self.coreiface.laddr, port=self.coreiface.lport, type='peer', proto='udp', laddr=self.coreiface.addr, lport=self.coreiface.port)
        self.clientiface = self.client.findiface(corename)
        if not self.core.name in Join.set:
            Join.set[self.core.name] = dict()
        if not self.client.name in Join.set:
            Join.set[self.client.name] = dict()
        Join.set[self.core.name][self.coreiface.id] = self
        Join.set[self.client.name][self.clientiface.id] = self
        printc('New join between ' + str(self.client.name) + ' and ' + str(self.core.name) , "cyan")

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'client='
        com += printstr(self.client.name)
        com += ',core='
        com += printstr(self.core.name)
        com += ')'
        return com

    def __repr__(self):
        return str(self)

    def gstr(self):
        '''Graph description'''
        com = str(self.client.name) + '--' + str(self.core.name)
        com += ' [style=dotted,fontsize=8,fontcolor=blue,decorate=true,labelfloat=false,constraint=true'
        com += '];'
        return com

    def running(self):
        '''Indicates if each part of the link is running'''
        return self.core.running() and self.client.running() and self.coreiface.activ and self.clientiface.activ

    def delete(self):
        '''Deletes the link'''
        printc('Unjoining ' + str(self.client.name) + ' and ' + str(self.core.name), 'cyan')
        self.client.unsetiface(self.clientiface.id)
        self.core.unsetiface(self.coreiface.id)
        self.client.deliface(self.clientiface.id)
        self.core.deliface(self.coreiface.id)
        Join.set[self.client.name][self.clientiface.id] = None
        Join.set[self.core.name][self.coreiface.id] = None


def Unjoin(*targets):
    '''Unjoin(targets)
    - Def -
        Unjoin disables the connection of an air virtual interface.
    - Args -
        . targets [list of str] --> <VAirLink name>[:<VAirLink name>]
    '''
    for v in targets:
        name = v
        iface = None
        vobj = None
        viface = None
        if ':' in v:
            iface = str(v.split(':')[1])
            name = str(v.split(':')[0])
        if name in VAirLink.set:
            vobj = VAirLink.set[name]
            if iface:
                viface = vobj.findiface(iface)
        else:
            return
        if name in Join.set:
            if viface:
                if viface.id in Join.set[name] and Join.set[name][viface.id]:
                    Join.set[name][viface.id].delete()
            else:
                if Join.set[name]:
                    for i in Join.set[name]:
                        if Join.set[name][i]:
                            Join.set[name][i].delete()

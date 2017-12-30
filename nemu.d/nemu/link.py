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

'''This module manages link between virtual elements'''

from nemu.error import NemuError
from nemu.name import NemuNameSpace
from nemu.msg import (printc, printstr)

class Link():
    '''Link(client, core)
    - Def -
        Link enables a connection between a set virtual interface and another unset virtual interface.
    - Args -
        . client [str] --> <VLink|VNode name>[:<niface>]
        . core [str] --> <VLink|VNode name>[:<niface>]
    '''
    set = dict()
    def __init__(self, client, core):
        corename = core
        clientname = client
        coreiface = 0
        clientiface = 0
        if ':' in core:
            coreiface = str(core.split(':')[1])
            corename = str(core.split(':')[0])
        if ':' in client:
            clientiface = str(client.split(':')[1])
            clientname = str(client.split(':')[0])
        try:
            self.core = NemuNameSpace.set[corename]
        except KeyError:
            raise NemuError('Cannot find ' + str(corename))
        try:
            self.client = NemuNameSpace.set[clientname]
        except KeyError:
            raise NemuError('Cannot find ' + str(clientname))
        
        self.coreiface = self.core.findiface(coreiface)
        self.clientiface = self.client.findiface(clientiface)

        if not self.core.name in Link.set:
            Link.set[self.core.name] = dict()
        if not self.client.name in Link.set:
            Link.set[self.client.name] = dict()
        if self.coreiface.id not in Link.set[self.core.name]:
            Link.set[self.core.name][self.coreiface.id] = None
        if self.clientiface.id not in Link.set[self.client.name]:
            Link.set[self.client.name][self.clientiface.id] = None

        if Link.set[self.client.name][self.clientiface.id]:
            Link.set[self.client.name][self.clientiface.id].delete()
        if Link.set[self.core.name][self.coreiface.id]:
            Link.set[self.core.name][self.coreiface.id].delete()

        if self.clientiface.activ:
            self.client.unsetiface(clientiface)
        if self.coreiface.activ and not self.coreiface.type == 'srv':
            self.core.unsetiface(coreiface)
        if not self.coreiface.activ:
            self.core.setiface(coreiface, addr=None, port=None, type='srv', proto=None, lport=None, laddr=None)

        Link.set[self.client.name][self.clientiface.id] = self
        Link.set[self.core.name][self.coreiface.id] = self
        
        if self.core.type == 'slirp':
            self.client.slirpiface(self.clientiface.id, self.coreiface.net, self.coreiface.restrict, self.coreiface.forwards, 'cli')
        else:
            self.client.setiface(self.clientiface.id, self.coreiface.addr, self.coreiface.port, 'cli', self.coreiface.proto, self.coreiface.laddr, self.coreiface.lport)
        printc("New link between " + str(self.client.name) + ":" + str(self.clientiface.id) + " and " + str(self.core.name) + ":" + str(self.coreiface.id), "cyan")

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'client='
        com += printstr(self.client.name + ':' + str(self.clientiface.id))
        com += ',core=' 
        com += printstr(self.core.name + ':' + str(self.coreiface.id))
        com += ')'
        return com

    def __repr__(self):
        return str(self)
        
    def gstr(self):
        '''Graph description'''
        com = str(self.client.name) + '--' + str(self.core.name)
        com += ' [style=dashed,fontsize=8,fontcolor=blue,decorate=true,labelfloat=false,constraint=true'
        com += ',taillabel="iface->' + str(self.clientiface.id) + '"'
        com += ',headlabel="iface->' + str(self.coreiface.id) + '"'
        com += '];'
        return com

    def running(self):
        '''Indicates if each part of the link is running'''
        return self.core.running() and self.client.running() and self.coreiface.activ and self.clientiface.activ
    
    def start(self):
        '''Starts the link'''
        if self.running():
            printc('Starting link between ' + str(self.client.name) + ":" + str(self.clientiface.id) + ' and ' + str(self.core.name) + ":" + str(self.coreiface.id), 'cyan')
            self.client.unsetlink(self.clientiface.id)
            self.client.setlink(self.clientiface.id)
        
    def stop(self):
        '''Stops the link'''
        if self.client.running():
            printc('Stopping link between ' + str(self.client.name) + ":" + str(self.clientiface.id) + ' and ' + str(self.core.name) + ":" + str(self.coreiface.id), 'cyan')
            self.client.unsetlink(self.clientiface.id)
    
    def delete(self):
        '''Deletes the link'''
        self.stop()
        printc('Unlinking ' + str(self.client.name) + ":" + str(self.clientiface.id) + ' and ' + str(self.core.name) + ":" + str(self.coreiface.id), 'cyan')
        self.client.unsetiface(self.clientiface.id)
        Link.set[self.client.name][self.clientiface.id] = None
        Link.set[self.core.name][self.coreiface.id] = None


def Unlink(*targets):
    '''Unlink(*targets)
    - Def -
        Unlink disables the connection of a virtual interface.
    - Args -
        . targets [list of str] --> <VNode name>[:<niface>] or <VLink name>[:<niface>]
    '''
    for v in targets:
        name = v
        iface = None
        vobj = None
        viface = None
        if ':' in v:
            iface = str(v.split(':')[1])
            name = str(v.split(':')[0])
        if name in NemuNameSpace.set:
            vobj = NemuNameSpace.set[name]
            if iface:
                viface = vobj.findiface(iface)
        else:
            raise NemuError('Cannot find ' + str(name))
        if name in Link.set:
            if viface:
                if viface.id in Link.set[name] and Link.set[name][viface.id]:
                    Link.set[name][viface.id].delete()
            else:
                if Link.set[name]:
                    for i in Link.set[name]:
                        if Link.set[name][i]:
                            Link.set[name][i].delete()

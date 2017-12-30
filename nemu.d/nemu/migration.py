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

'''This module contains functions to perform a node migration'''

from nemu.error import NemuError
from nemu.msg import printc
from nemu.vnode import VNode


def MigrateVNode(name, addr, port, incremental=False, speed='1g'):
    '''MigrateVNode(name, addr, port, incremental, speed)
    - Def -
        Performs a live migration of a VNode to another session.
    - Args -
        . name [str]         --> name of the desired virtual node
        . addr [str]         --> destination address of the remote host which is waiting for the incoming migration
        . port [str]         --> destination port of the remote virtual node which is waiting for the incoming migration
        . incremental [bool] --> increases the migration speed if the source and the destination virtual nodes share derived versions of the same VFs (Default: False)
        . speed [str]        --> migration speed in k|m|g (Default: 1g)
    '''
    try:
        node = VNode.set[name]
        printc('Starting migration of ' + str(name) + ' to ' + str(addr) + ':' + str(port), 'cyan')
        node.send('migrate_set_speed ' + str(speed))
        node.send('migrate -i tcp:' + str(addr) + ':' + str(port))
    except KeyError:
        raise NemuError('Cannot find the Virtual Node ' + str(i))


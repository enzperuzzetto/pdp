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

'''This module manages VRouter Services'''

import os
import sys
from nemu.error import NemuError
from nemu.msg import (printc, printstr)
from nemu.path import (mpath, copyd)

class Service:
    '''Service(name, *largs, **kargs)
    - Def -
        A Service is an optionnal feature for VRouters.
    - Args -
        . name [str] --> Service name
        . largs [list] --> Service list arguments (Default: empty)
	. kargs [dict] --> Service keyword arguments (Default: empty)
    '''
    def __init__(self, name, *largs, **kargs):
        self.name = name
        self.largs = largs
        self.kargs = kargs
        self.root = mpath(os.path.dirname(__file__), 'vrs', self.name)
        self.script = mpath(self.root, '__init__.py')
        self.tcz = mpath(self.root, 'tcz')
        self.files = mpath(self.root, 'files')
        if not os.path.isdir(self.root) or not os.path.isfile(self.script):
            raise NemuError('Cannot find the Service ' + str(self.name))
        printc(str(self.name) + ' loaded', 'cyan')

    def run(self, vrouter):
        '''Starts service'''
        printc('Setting up ' + str(self.name), 'cyan')
        com = 'import nemu.vrs.' + self.name
        exec(com)
        tcz = self.tcz
        if os.path.isdir(tcz):
            for i in os.listdir(tcz):
                if i[0] == '.':
                    continue
                copyd(mpath(tcz, i), mpath(vrouter.tcz, i))
        if os.path.isdir(self.files):
            copyd(self.files, vrouter.files)
        com = 'nemu.vrs.' + self.name + '.init' + '(vrouter, *self.largs, **self.kargs)'
        exec(com)

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += printstr(self.name)
        com += ',*' + printstr(self.largs)
        com += ',**' + printstr(self.kargs)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


class UserService(Service):
    '''Service(name, root, *largs, **kargs)
    - Def -
        A Service is an optionnal feature for VRouters.
    - Args -
        . name [str] --> Service name
        . root [str] --> Service directory path
        . largs [list] --> Service list arguments (Default: empty)
	. kargs [dict] --> Service keyword arguments (Default: empty)
    '''
    def __init__(self, name, root, *largs, **kargs):
        self.name = name
        self.largs = largs
        self.kargs = kargs
        self.root =  mpath(root, self.name)
        self.script = mpath(self.root, '__init__.py')
        self.tcz = mpath(self.root, 'tcz')
        self.files = mpath(self.root, 'files')
        if not os.path.isdir(self.root) or not os.path.isfile(self.script):
            raise NemuError('Cannot find the Service ' + str(self.name))
        printc(str(self.name) + ' loaded', 'cyan')

    def run(self, vrouter):
        '''Starts service'''
        printc('Setting up ' + str(self.name), 'cyan')
        if not os.path.dirname(self.root) in sys.path:
            sys.path.append(os.path.dirname(self.root))
        com = 'import ' + self.name
        exec(com)
        tcz = self.tcz
        if os.path.isdir(tcz):
            for i in os.listdir(tcz):
                if i[0] == '.':
                    continue
                copyd(mpath(tcz, i), mpath(vrouter.tcz, i))
        if os.path.isdir(self.files):
            copyd(self.files, vrouter.files)
        com = self.name + '.init' + '(vrouter, *self.largs, **self.kargs)'
        exec(com)
        
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += printstr(self.name)
        com += ',root=' + printstr(self.root)
        com += ',*' + printstr(self.largs)
        com += ',**' + printstr(self.kargs)
        com += ')'
        return com
	
def ListService(*which):
    '''ListService(*names)
    - Def -
        Help for several Services.
    - Args -
        . names [list of str] : names of desired services (Default: displays all)
    '''
    lret = list()
    services = which
    if len(which) == 0:
        services = os.listdir(mpath(os.path.dirname(__file__), 'vrs'))
    for i in sorted(services):
        try:
            ret = None
            com = 'import nemu.vrs.' + i
            exec(com)
            com = 'ret = nemu.vrs.' + i + '.help' + '()'
            exec(com)
            printc('-- ' + i + ' --')
            printc("\t" + ret['desc'] + '.')
            printc("\t" + ret['syn'])
            if 'args' in ret and len(ret['args']) > 0:
                printc( "\tArgs:")
                for k in ret['args']:
                    printc("\t\t. " + k)
            printc("")
            lret.append(i)
        except Exception:
            pass
    return lret


# ---------------------------------------------------------------------------
# -- NEmu : The Network Emulator for Mobile Universes --
# ---------------------------------------------------------------------------

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

'''This module contains the boostrap and help functions'''

__author__ = "Vincent Autefage"
__date__ = ""
__version__ = "0.6.3"
__version_info__ = (0, 6, 3)
__version_vnd__ = "0.5.6"
__version_nemo__ = "0.2.4"
__version_vrouter_initrd__ = "5.2"
__version_vrouter_kernel__ = "3.8.13-tinycore"
__license__ = "GNU Lesser General Public License (LGPL v3)"

import sys
import os

if sys.version_info < (2, 6):
    raise RuntimeError('NEmu is not fully compatible for python version under 2.6')

if sys.version_info > (3, 0):
    raise RuntimeError('NEmu is not fully compatible for python version above 2.x')

from nemu.error import NemuError
from nemu.hist import HistoryNemu
from nemu.vfs import (VFs, EmptyFs, ExportFs)
from nemu.nbd import (RemoteNBD, StartRemoteNBD, StopRemoteNBD, DelRemoteNBD)
from nemu.iface import (VNic, SetIface, UnsetIface, ConfIface, UnconfIface, DumpIface, UndumpIface, AddIface)
from nemu.vhost import (VHostConf, VHost)
from nemu.vrouter import (VRouterConf, VRouter, GenVRouter)
from nemu.session import (InitNemu, GetVar, SetVar, DelVar, GetSet)
from nemu.save import (SaveNemu, RestoreNemu)
from nemu.link import (Link, Unlink)
from nemu.vnd import (VLine, VHub, VSwitch)
from nemu.vde import (VDEHub, VDESwitch)
from nemu.vremote import VRemote
from nemu.vslirp import VSlirp
from nemu.vtap import VTap
from nemu.vqhub import VQHub
from nemu.forward import Forward
from nemu.graph import (GraphNemu, SubGraphNemu)
from nemu.delete import (DelVNode, DelVLink, DelNemu)
from nemu.conf import (ExportNemu)
from nemu.sched import (StartVNode, StopVNode, RebootVNode, StartVLink, StopVLink, RebootVLink, WaitNemu, StartNemu, StopNemu, RebootNemu)
from nemu.remote import (RemoteNemu, StartRemoteNemu, ShellRemoteNemu, ComRemoteNemu, FileRemoteNemu, MountRemoteNemu, UmountRemoteNemu, StopRemoteNemu, DelRemoteNemu)
from nemu.proc import ComProcNemu
from nemu.service import (Service, UserService, ListService)
from nemu.vair import (VAirWic, VAirAp, VAirRemote, SetAirMode, SetAirIface)
from nemu.join import (Join, Unjoin)
from nemu.vrc import VRc
from nemu.mobile import (MobNemu, GenMobNemu, ExportMobNemu, ImportMobNemu, FreezeMobNemu, ResumeMobNemu, StartMobNemu, StopMobNemu, DelMobNemu)
from nemu.migration import MigrateVNode

def plop():
    '''Plop'''
    print "plop"

def PlopNemu():
    '''PlopNemu()
    - Def -
        Starts NEmu internal threads (required only for module mode).
    '''
    import nemu.loop
    import nemu.msg
    try:
        nemu.loop.process_loop()
    except NemuError as e:
        nemu.msg.printc(str(e), 'magenta')
        pass
        
def HelpNemu(*which):    
    '''HelpNemu(*which)
    - Def -
        Help. What else ?...
    - Args -
        . which [list of NEmu Objects]
    '''
    def _help_default_form():
        '''func(args...)
        - Def -
            Definition
        - Args -
            . arg [type] --> informations
            . ...
        '''  
    
    hpl = [PlopNemu, InitNemu, SaveNemu, RestoreNemu, ExportNemu, ImportNemu, GraphNemu, HistoryNemu, StartNemu, StopNemu, RebootNemu, WaitNemu,
	   VFs, EmptyFs, ExportFs, VHostConf, RemoteNBD, StartRemoteNBD, StopRemoteNBD, DelRemoteNBD, VHost, VRouterConf, VRouter, GenVRouter, Service, UserService, ListService, VNic, 
           VLine, VHub, VSwitch, VRemote, VSlirp, VTap, VQHub, VDEHub, VDESwitch, SetIface, UnsetIface, AddIface, ConfIface, UnconfIface, DumpIface, UndumpIface, Link, Unlink, 
           VAirWic, VAirAp, VAirRemote, SetAirMode, SetAirIface, Join, Unjoin, Forward,
	   StartVNode, StopVNode, RebootVNode, StartVLink, StopVLink, RebootVLink, DelVNode, DelVLink, DelNemu, MigrateVNode,
           MobNemu, GenMobNemu, ExportMobNemu, ImportMobNemu, FreezeMobNemu, ResumeMobNemu, StartMobNemu, StopMobNemu, DelMobNemu,
	   RemoteNemu, StartRemoteNemu, ShellRemoteNemu, ComRemoteNemu, FileRemoteNemu, MountRemoteNemu, UmountRemoteNemu, StopRemoteNemu, DelRemoteNemu, ComProcNemu,
	   GetVar, SetVar, DelVar, GetSet, HelpNemu, UseNemu, VerNemu]
    if len(which) > 0:
        this = list(which)
        for i in this:
            print ' -- ' + i.__doc__
        return
    
    print '------------------------------------------------------'
    print '-- NEmu - The Network Emulator for Mobile Universes --'
    print '------------------------------------------------------'
    print ''
    print 'SYNOPSIS'
    print '\tnemu [-i|--interactive]'
    print '\tnemu [-f|--file] <config file>'
    print '\tnemu [-r|--remote] <user>@<host>[:<port>[:<path>]]'
    print '\tnemu [-h|--help]'
    print '\tnemu [-u|--usage]'
    print '\tnemu [-v|--version]'
    print ''
    print ''
    print 'DESCRIPTION'
    print '\tNEmu is a distributed virtual network environment.'
    print '\tIt provides self-script (launcher and interpreter) and a python API for creating heterogeneous, dynamic, distributed, collaborative and mobile virtual networks on the top of QEMU.'
    print ''
    print ''
    print 'CONFIG FILE SYNTAX'
    print '\tA NEmu configuration file is written with a python syntax.'
    print ''
    print ''
    for i in hpl:
        print ' -- ' + i.__doc__
    print ''
    print 'AUTHOR'
    print '\tWritten by ' + str(__author__) + '.'
    print ''
    print 'CONTACT & REPORTING BUGS'
    print '\thttp://nemu.valab.net'
    print ''
    print 'COPYRIGHT'
    print '\t(c) 2011-2016 ' + str(__author__) + '.'
    print '\t' + str(__license__)
    print ''
    print 'SEE ALSO'
    print '\tqemu'

def VerNemu():
    '''VerNemu()
    - Def -
        Software version
    '''
    print 'NEmu - The Network Emulator for Mobile Universes'
    print ''
    print 'Version: ' + str(__version__)
    print '\t' + '. vnd : ' + str(__version_vnd__)
    print '\t' + '. nemo : ' + str(__version_nemo__)
    print '\t' + '. vrouter initrd : ' + str(__version_vrouter_initrd__)
    print '\t' + '. vrouter kernel : ' + str(__version_vrouter_kernel__)
    print 'Author: ' + str(__author__)
    print 'License: ' + str(__license__)
    print 'http://nemu.valab.net'
    print '(c) 2011-2016 ' + str(__author__)
# ------------------------------------------------------------------------------------------------------------

def UseNemu():
    '''UseNemu()
    - Def -
        Software usage
    '''
    print 'Usage :\t' + os.path.basename(sys.argv[0]) + ' [[-i|--interactive]'    
    print '       \t' + os.path.basename(sys.argv[0]) + ' [[-f|--file] <config file>]'    
    print '       \t' + os.path.basename(sys.argv[0]) + ' [-r|--remote] <user>@<host>[:<port>[:<path>]]'
    print '       \t' + os.path.basename(sys.argv[0]) + ' [-u|--usage]'
    print '       \t' + os.path.basename(sys.argv[0]) + ' [-v|--version]'

    print 'Please ' + os.path.basename(sys.argv[0]) + ' [-h|--help] for more information.'
    return 1


def ImportNemu(src):
    '''ImportNemu(src)
    - Def -
        Imports a configuration script in the current NEmu instance.
    - Args -
        . src [str] --> config file
    '''
    import nemu.msg
    nemu.msg.printc('Configuration from', 'blue')
    nemu.msg.printc(str(src), 'cyan')
    import nemu.loop
    nemu.loop.main_loop(str(src))


def nemu_main():
    '''NEmu start point'''
    if len(sys.argv) >= 2:
        if sys.argv[1] == '-h' or sys.argv[1] == '--help':
            HelpNemu()
            return 0
        elif sys.argv[1] == '-v' or sys.argv[1] == '--version':
            VerNemu()
            return 0
        elif sys.argv[1] == '-u' or sys.argv[1] == '--usage':
            UseNemu()
            return 0

    where_file = None
    where_remote = None

    import nemu.loop
    import nemu.msg
    nemu.msg.basebann()
    nemu.loop.process_loop()
    if '--remoted' in sys.argv:
        sys.ps1 = nemu.msg.baseps1(sys.argv[sys.argv.index('--remoted')+1])
    else:
        print nemu.msg.basebann()
        sys.ps1 = nemu.msg.baseps1()
        try:
            import readline
            import rlcompleter
            readline.parse_and_bind("tab: complete")
        except ImportError:
            pass
    if '-f' in sys.argv or '--file' in sys.argv:
        if '-f' in sys.argv:
            where_file = sys.argv.index('-f') + 1
        else:
            where_file = sys.argv.index('--file') + 1
    if '-r' in sys.argv or '--remote' in sys.argv:
        if '-r' in sys.argv:
            where_remote = sys.argv.index('-r') + 1
        else:
            where_remote = sys.argv.index('--remote') + 1
    if where_remote:
        interactive = ('-i' in sys.argv or '--interactive' in sys.argv)
        if where_file:
            nemu.loop.remote_interactive_loop(sys.argv[where_remote], sys.argv[where_file], interactive)
        else:
            nemu.loop.remote_interactive_loop(sys.argv[where_remote])
        sys.exit(0)
    elif where_file:
        nemu.loop.main_loop(sys.argv[where_file])
        if not '-i' in sys.argv and not '--interactive' in sys.argv:
            sys.exit(0)

    nemu.loop.main_loop()
    sys.exit(0)


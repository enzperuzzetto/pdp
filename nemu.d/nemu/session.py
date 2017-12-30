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

'''This module manages sessions'''

import os
import re
import tempfile
import atexit

from nemu.msg import (printok, printc, printlog, printstr)
from nemu.proc import killprocl
from nemu.hist import (restore_nemu_history)
from nemu.path import (mpath, symlink)
from nemu.var import NemuVar


def _nemu_end():
    '''Nemu end handler'''
    killprocl()
    printlog('--------- SESSION STOPPED ---------\n')

class InitNemu:
    '''InitNemu(session, workspace, hdcopy, color, quiet)
    - Def -
        Creates a new NEmu session (required before creating anything else). The session root directory is set to <workspace>/<session>.
    - Args -
        . session [str] --> session name (Default: Autoset)
        . workspace [str] --> session parent directory (Default: /tmp) 
        . hdcopy [bool] --> hard copy for drive images in the <session> (Default: True)
        . color [bool]  --> enables/disables text colorization (Default: True)
        . quiet [bool]  --> disables the display of NEmu messages (Default: False)
    '''
    def __init__(self, session=None, workspace=mpath('/tmp'), hdcopy=True, color=True, quiet=False):
        try:
            NemuVar.nemud = os.environ['NEMUROOT']
        except:
            pass
        NemuVar.nemurcd = mpath(NemuVar.nemud, NemuVar.nemurcd)
        NemuVar.hdcopy = hdcopy
        NemuVar.workspace = os.path.abspath(workspace)
        NemuVar.color = color
        NemuVar.quiet = quiet
        if not os.path.exists(NemuVar.kvm):
            printc('KVM module does not seem to be loaded', 'magenta')
        if not session == None:
            NemuVar.rootd = mpath(NemuVar.workspace, session)
            if not os.path.isdir(NemuVar.rootd):
                os.mkdir(NemuVar.rootd, 0700)
        else:
            NemuVar.rootd = tempfile.mkdtemp(prefix='nemu.session.', dir=NemuVar.workspace)
        NemuVar.session = os.path.basename(NemuVar.rootd)
        NemuVar.dotf = mpath(NemuVar.rootd, NemuVar.dotf)
        NemuVar.logf = mpath(NemuVar.rootd, NemuVar.logf)
        NemuVar.conff = mpath(NemuVar.rootd, NemuVar.conff)
        NemuVar.histf = mpath(NemuVar.rootd, NemuVar.histf)
        logfd = open(NemuVar.logf, 'a+')
        logfd.close()
        atexit.register(_nemu_end)
        printlog('--------- SESSION LOADED ---------')
        printc('Init session in', 'blue')
        printc(NemuVar.rootd, 'cyan')
        printc('Init log in', 'blue')
        printc(NemuVar.logf, 'cyan')
        NemuVar.fsd = mpath(NemuVar.rootd, NemuVar.fsd)
        NemuVar.rcd = mpath(NemuVar.rootd, NemuVar.rcd)
        NemuVar.rcond = mpath(NemuVar.rootd, NemuVar.rcond)
        NemuVar.mobd = mpath(NemuVar.rootd, NemuVar.mobd)
        NemuVar.noded = mpath(NemuVar.rootd, NemuVar.noded)
        NemuVar.linkd = mpath(NemuVar.rootd, NemuVar.linkd)
        NemuVar.tapup = mpath(NemuVar.nemurcd, 'tools', 'tap-ifup')
        NemuVar.tapdown = mpath(NemuVar.nemurcd, 'tools', 'tap-ifdown')
        if not os.path.isdir(NemuVar.fsd):
            os.mkdir(NemuVar.fsd)
        if not os.path.isdir(NemuVar.rcd):
            os.mkdir(NemuVar.rcd)
        if not os.path.isdir(NemuVar.rcond):
            os.mkdir(NemuVar.rcond)
        if not os.path.isdir(NemuVar.mobd):
            os.mkdir(NemuVar.mobd)
        if not os.path.isdir(NemuVar.noded):
            os.mkdir(NemuVar.noded)
        if not os.path.isdir(NemuVar.linkd):
            os.mkdir(NemuVar.linkd)
        NemuVar.lnf = 'symlink.' + os.path.basename(NemuVar.rootd)
        printc('Link point is', 'blue')
        printc(NemuVar.lnf, 'cyan')
        if not os.path.islink(NemuVar.lnf):
            try:
                symlink(NemuVar.rootd, NemuVar.lnf)
            except OSError:
                printc('Cannot create ' + NemuVar.lnf, 'magenta')
        restore_nemu_history()
        histfd = open(NemuVar.histf, 'a+')
        histfd.close()
        NemuVar.this = self
        printok()

    def qstr(self):
        '''Shell script head'''
        com = '#!/bin/sh'
        return com

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'session=' + printstr(NemuVar.session)
        com += ',workspace=' + printstr(NemuVar.workspace)
        com += ',hdcopy=' + printstr(NemuVar.hdcopy)
        com += ',color=' + printstr(NemuVar.color)
        com += ',quiet=' + printstr(NemuVar.quiet)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


def SetVar(**kargs):
    '''SetVar(**kargs)
    - Def -
        Sets several NEmu properties.
    - Args -
        . kargs [dict] --> properties name and value (key=val)
    '''
    for key in kargs.keys():
        val = kargs[key]
        ret = str(key) + "=" + printstr(val)
        try:
            setattr(NemuVar, str(key), val)
            printc(ret, 'cyan')
        except Exception:
            printc('Cannot set ' + str(key) + ' to ' + str(val), 'magenta')
            pass

def GetVar(*largs):
    '''GetVar(*largs)
    - Def -
        Gets several NEmu properties.
    - Args -
        . largs [list of str] --> properties name (default: all)
    '''
    lret = dict()
    if len(largs) == 0:
        largs = list()
        for key in dir(NemuVar):
            if not re.match('__.*__', key):
                largs.append(str(key))
    for key in largs:
        val = None
        try:
            val = getattr(NemuVar, str(key))
            printc(str(key) + "=" + printstr(val), 'cyan')
            lret[key] = val
        except Exception:
            printc('Cannot get ' + str(key), 'magenta')
            pass
    return lret

def DelVar(*largs):
    '''DetVar(*largs)
    - Def -
        Deletes several NEmu properties.
    - Args -
        . largs [list of str] --> properties name
    '''
    for key in largs:
        try:
            delattr(NemuVar, str(key))
            printc(str(key) + " deleted", 'cyan')
        except Exception:
            printc('Cannot get ' + str(key), 'magenta')
            pass

def GetSet(obj):
    '''GetSet(obj)
    - Def -
        Gets the set of same NEmu objects.
    - Args -
        . objs [str] --> name of the NEmu object
    '''
    ret = dict()
    try:
        ret.update(getattr(obj, 'set'))
    except Exception:
        raise NemuError(str(obj) + ' has no set')
    for key in ret.keys():
        printc(str(key), 'cyan')
    return ret

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

'''This module manages sessions save/restore process'''

import os
import tarfile

from nemu.msg import (printc, printok)
from nemu.error import NemuError
from nemu.var import NemuVar
from nemu.conf import (ExportNemu)
from nemu.path import mpath


def SaveNemu(dest=None):
    '''SaveNemu(dest)
    - Def -
        Saves a NEmu session in a sparse tgz archive. Internal configutation script and history are also saved in the archive.
    - Args -
        . dest [str] --> name of the sparse archive (Default: <session>.tgz)
    '''
    ExportNemu(NemuVar.conff)
    if dest == None:
        dest = os.path.basename(NemuVar.rootd) +  '.tgz'
    printc('Saving NEmu session in ' + str(dest), 'blue')
    tar = tarfile.open(dest, "w:gz")
    olddir = os.getcwd()
    os.chdir(NemuVar.workspace)
    tar.add(os.path.basename(NemuVar.rootd))
    os.chdir(olddir)
    tar.close()
    printok()

def RestoreNemu(path, workspace=mpath('/tmp')):
    '''RestoreNemu(session, workspace)
    - Def -
        Restores a previous NEmu session from a sparse tgz archive.
    - Args -
        . path [str] --> path to the session archive
        . workspace [str] --> workspace of the session (Default: /tmp)
    '''
    if not tarfile.is_tarfile(path):
        raise NemuError(path + ' is not a tarfile')
    printc('Restoring NEmu session from ' + str(path), 'blue')
    tar = tarfile.open(path, "r:gz")
    tar.extractall(path=workspace)
    tar.close()
    printok()

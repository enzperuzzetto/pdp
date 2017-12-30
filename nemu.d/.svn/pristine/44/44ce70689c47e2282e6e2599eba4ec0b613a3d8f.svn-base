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

'''This module provides several path functions'''

import os
import ctypes
import sys
import shutil

def mpath(*path):
    '''Concatenates path with a correct shell syntax'''
    return os.path.normpath(os.path.normcase(os.path.join(*path)))

def symlink(src, dest):
    '''Makes a symlink with the correct os method'''
    if sys.platform == "win32":
        kwindll = ctypes.windll.LoadLibrary("kernel32.dll")
        kwindll.CreateSymbolicLinkA(src, dest, 0)
    else:
        os.symlink(src, dest)

def copyd(src, dest):
    '''Copy tree UNIX implementation'''
    if os.path.isdir(src):
        if os.path.isdir(dest): 
            for i in os.listdir(src):
                copyd(mpath(src, i), mpath(dest, i))
        else:
            shutil.copytree(src, dest)
    else:
        shutil.copyfile(src, dest)

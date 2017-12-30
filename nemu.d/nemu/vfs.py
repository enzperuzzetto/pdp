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

'''This module manages virtual file systems'''

import os
import shutil
import tempfile

from nemu.msg import (printc, printstr, printok)
from nemu.proc import actproc
from nemu.error import NemuError
from nemu.var import NemuVar
from nemu.path import (mpath, symlink)
from nemu.nbd import nbdvfs


class VFs():
    '''VFs(path, type, tag, mode, **opts)
    - Def -
        Returns a format path from a real directory path and could be used in the hds list.
    - Args -
        . path [str] --> desired path
        . type [str] --> 'raw', 'cow', 'link', 'copy', 'vfat', 'virtio', 'nbd', 'squash', 'ssh', 'http', 'https', 'ftp', 'ftps', 'tftp' (Default: copy)
        . tag [str] --> mount tag (Default: basename of path)
        . mode [str] --> "ro" or "rw" (Default: rw)
        . opts [dict] --> other specific type options (Default: empty)
    '''
    def __init__(self, path, type='raw', tag=None, mode='rw', **opts):
        self.orig = path
        self.type = type
        self.mode = mode
        self.opts = dict()
        self.opts.update(opts)
        self.tag = tag
        if self.type == 'raw':
            self.path = self.orig
            self.tag = os.path.basename(self.orig)
            printc('Importing the raw FS ' + str(self.orig), 'cyan')
        elif self.type == 'copy':
            if not self.tag:
                self.tag = os.path.basename(self.orig)
                self.path = tempfile.mkstemp(prefix='__', suffix='__.' + self.tag, dir=NemuVar.fsd)[1]
                self.tag = os.path.basename(self.path)
                os.unlink(self.path)
            self.path = mpath(NemuVar.fsd, self.tag)
            printc('Importing a copy from the FS ' + str(self.orig), 'cyan')
            if not os.path.lexists(self.path):
                printc(self.orig + ' --> ' + str(self.path), 'base')
                shutil.copyfile(self.orig, self.path)
            else:
                printc(str(self.path) + ' already exists', 'magenta')
        elif self.type == 'link':
            if not self.tag:
                self.tag = os.path.basename(self.orig)
                self.path = tempfile.mkstemp(prefix='__', suffix='__.' + self.tag, dir=NemuVar.fsd)[1]
                self.tag = os.path.basename(self.path)
                os.unlink(self.path)
            self.path = mpath(NemuVar.fsd, self.tag)
            printc('Importing a link to the FS ' + str(self.orig), 'cyan')
            if not os.path.lexists(self.path):
                printc(self.orig + ' --> ' + str(self.path), 'base')
                symlink(self.orig, self.path)
            else:
                printc(str(self.path) + ' already exists', 'magenta')
        elif self.type == 'cow':
            vroot = None
            if NemuVar.hdcopy:
                vroot = VFs(self.orig, 'copy', os.path.basename(self.orig))
            else:
                vroot = VFs(self.orig, 'link', os.path.basename(self.orig))
            self.orig = vroot.path
            if self.tag:
                self.path = mpath(NemuVar.fsd, self.tag)
            else:
                self.tag = os.path.basename(self.orig)
                self.path = tempfile.mkstemp(prefix='__', suffix='__.' + self.tag, dir=NemuVar.fsd)[1]
                self.tag = os.path.basename(self.path)
                os.unlink(self.path)
            if not os.path.isfile(self.path):
                printc('Generating a qcow2 from ' + str(self.orig), 'cyan')
                com = 'qemu-img create -f qcow2 -b ' + str(self.orig) + ' ' + str(self.path)
                actproc(com)
            else:
                printc(str(self.path) + ' already exists', 'magenta')
        elif self.type == 'squash':
            if not self.tag:
                self.tag = os.path.basename(self.orig)
                self.path = tempfile.mkstemp(prefix='__', suffix='__.' + self.tag, dir=NemuVar.fsd)[1]
                self.tag = os.path.basename(self.path)
                os.unlink(self.path)
            self.path = mpath(NemuVar.fsd, self.tag)
            printc('Generating a squashFS image from ' + str(self.orig), 'cyan')
            if not os.path.isfile(self.path):
                actproc('mksquashfs ' + str(self.orig) + ' ' + str(self.path))
            else:
                printc(str(self.path) + ' already exists', 'magenta')
        elif self.type == 'vfat':
            printc('Generating a vfat from ' + str(self.orig) + ' [' + str(self.mode) + ']', 'cyan')
            if self.mode == 'ro':
                self.path = 'fat:' + ':' + str(self.orig)
            elif self.mode == 'rw':
                self.path = 'fat:' + str(self.mode) + ':' + str(self.orig)
            else:
                raise NemuError(str(self.mode) + ' is not a valid mode')
        elif self.type == 'virtio':
            printc('Generating a virtio from ' + str(self.orig) + ' with the tag ' + str(self.tag), 'cyan')
            if self.mode == 'rw':
                cmode = ''
            elif self.mode == 'ro':
                cmode = ',readonly'
            else:
                raise NemuError(str(self.mode) + ' is not a valid mode')
            if not self.tag:
                self.tag = os.path.basename(self.orig)	
            self.path = '-virtfs '
            if 'fsdriver' in self.opts:
                self.path += str(self.opts['fsdriver'])
            else:
                self.path += 'local'
            self.path += ',path=' + str(self.orig) + ',mount_tag=' + str(self.tag) + cmode
            if 'security_model' in self.opts:
                self.path += ',security_model=' + str(self.opts['security_model'])
            else:
                self.path += ',security_model=' + 'none'
            if 'writeout' in self.opts:
                self.path += ',writeout=' + str(self.opts['writeout'])
        elif self.type == 'ssh':
            printc('Generating a ssh from ' + str(self.orig), 'cyan')
            self.path = 'ssh://' + str(self.opts['user']) + '@' + str(self.orig)
        elif self.type == 'http' or self.type == 'https' or self.type == 'ftp' or self.type == 'ftps' or self.type == 'tftp':
            printc('Generating a ' + self.type  + ' from ' + str(self.orig), 'cyan')
            self.path = self.type + '://'
            if 'user' in self.opts:
                self.path += str(self.opts['user'])
                if 'password' in self.opts:
                    self.path += ':' + str(self.opts['password'])
                self.path += '@'
            self.path += str(self.orig)
        elif self.type == 'nbd':
            printc('Importing the NBD ' + str(self.orig), 'cyan')
            self.path = nbdvfs(self.orig)
        else:
            raise NemuError(str(self.type) + ' is not a valid VFs type')

    def qstr(self):
        '''QEMU string'''
        return self.path
            
    def __str__(self):
        com = self.__class__.__name__ +'('
        com += 'path=' + printstr(self.orig)
        com += ',type=' + printstr(self.type)
        com += ',tag=' + printstr(self.tag)
        com += ',mode=' + printstr(self.mode)
        com += ',**' + printstr(self.opts)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


def mkfsCom(dest, format, **opts):
    com = ''
    if format == "ext2" or format == "ext3" or format == "ext4":
        com = 'mke2fs -t ' + str(format) + ' -F ' + str(dest) + ' '
    elif format == "fat" or format == "vfat" or format == "msdos":
        com = 'mkfs.' + str(format) + ' ' +  str(dest) + ' '
    elif format == "ntfs":
        com = 'mkfs.' + str(format) + ' -F ' +  str(dest) + ' '
    elif format == "reiser4":
        com = 'mkfs.' + str(format) + ' -y -f ' +  str(dest) + ' '
    elif format == "minix" or format == "exfat" or format == "btrfs" or format == "hfs" or format == "hfsplus" or format == "nilfs2" or format == "ocfs2" or format == "xfs":
        com = 'mkfs.' + str(format) + ' ' +  str(dest) + ' '
    else:
        raise NemuError('Unknown format ' + str(format))
    keys = sorted(opts.keys())
    for i in keys:
        if isinstance(opts[i], list):
            for j in opts[i]:
                com += '-' + str(i).replace('_', '-') + ' ' + str(j) + ' '
        else:
            com += '-' + str(i).replace('_', '-') + ' '
        if not opts[i] == None:
            com += str(opts[i]) + ' '
    return com
            
            


def EmptyFs(name, size, type='raw', format=None, **opts):
    '''EmptyFs(name, size, type, format)
    - Def -
        Builds an empty <type>/<format> file system and returns a new VFs which could be used in the hds list.
    - Args -
        . name [str] --> name of the new empty file system
	. size [str] --> <x>[K|M|G|T]
	. type [str] --> qemu file format: raw | cow | qcow | qcow2 | qed | vdi | vmdk | vpc | cloop | ... (Default: raw)
	. format [str] --> ext2 | ext3 | ext4 | fat | vfat | exfat | msdos | ntfs | reiser4 | minix | btrfs | hfs | hfsplus | nilfs2 | ocfs2 | xfs (Default: None)
        . opts [dict] --> other format options. a=1, b_c="x", d=["y","z"] -> mkfs.<format> -a 1 -b-c x -d y -d z... (Default: empty)
    '''
    printc('Generating an empty image ' + str(name) + ' as a ' + str(type) + ' image of size ' + str(size), 'blue')
    dest = mpath(NemuVar.fsd, name)
    if not os.path.isfile(dest):
        actproc('qemu-img create -f ' + str(type) + ' ' + str(dest) + ' ' + str(size))
        if not format == None:
            printc('Formatting ' + str(name) + ' in ' + str(format), 'cyan')
            actproc(mkfsCom(dest, format, **opts))
    else:
        printc(str(name) + ' already exists', 'magenta')
    printok()
    return VFs(dest, 'raw')


def ExportFs(name, dest=None, type=None):
    '''ExportFs(name, dest, type)
    - Def -
        Exports a VFs from the current session.
    - Args -
        . name [str] --> name of the file system
        . dest [str] --> destination of the copy (Default: equal to name)
        . type [str] --> copy qemu file format (Default: None)
    '''
    if not dest:
        dest = str(name)
    src = mpath(NemuVar.fsd, str(name))
    printc('Exporting the image ' + str(src) + ' to ' + str(dest), 'blue')
    if os.path.lexists(src):
        if type:
            printc('Converting the image ' + str(src) + ' to ' + str(dest) + ' in ' + str(type), 'cyan')
            com = 'qemu-img convert ' + str(src) + ' -O ' + str(type) + ' ' + str(dest)
            if type == "qcow" or type == "qcow2":
                com += ' -c'
            actproc(com)
        else:
            shutil.copyfile(src, dest)
    else:
        raise NemuError('Cannot find ' + str(src))
    printok()


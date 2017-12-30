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

'''This module manages remote connections'''

import termios
import tty
import sys
import os
import select
import shutil

from nemu.error import NemuError
from nemu.name import NemuNameSpace
from nemu.msg import (printc, printok, printstr, printlog)
from nemu.path import mpath
from nemu.var import NemuVar
from nemu.proc import (NemuProc, actproc, popenproc, killprocl)


def isremoted():
    '''Indicates if NEmu is in remote mode'''
    return '--remoted' in sys.argv

class RemoteNemu():
    '''RemoteNemu(name, user, host, port, path, **opts)
    - Def -
   	Creates a NEmu remote connection configuration through SSH.
    - Args -
	. name [str]  --> name of the remote connection
	. user [str]  --> remote username for SSH
	. host [str]  --> remote hostname for SSH
	. port [str]  --> remote port for SSH (Default: 22)
	. path [str]  --> go to path before exec remote nemu (Default: None)
        . opts [dict] --> ssh options a=1, b="x", ssh -o a=1 -o b=x... (Default: empty)
     '''
    set = dict()
    def __init__(self, name, user, host, port=22, path=None, **opts):
        if name in RemoteNemu.set:
            raise NemuError('Remote connection ' + str(name) + ' already exists')
        if name in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        printc('Setting up the remote connection ' + str(name), 'blue')
        self.name = name
        self.user = user
        self.host = host
        self.port = port
        self.path = path
        self.buff = list()
        self.proc = None
        self.procfs = None
        self.launch = "$NEMUROOT/nemu.py --remoted nemu:" + str(self.name) + '; exit'
        if self.path:
            self.launch = "cd " + str(self.path) + ";" + str(self.launch)
        self.vard = mpath(NemuVar.rcond, str(self.name))
        self.sshfs =  mpath(self.vard, str(self.name) + '.sshfs')
        self.infile = mpath(self.vard, NemuVar.inlog)
        self.logfile = mpath(self.vard, NemuVar.outlog)
        if not os.path.isdir(self.vard):
            os.mkdir(self.vard)
        if not os.path.isdir(self.sshfs):
            os.mkdir(self.sshfs)
        infd = open(self.infile, 'a+')
        infd.close()
        outfd = open(self.logfile, 'a+')
        outfd.close()
        self.sshsession = None
        self.opts = dict()
        self.sshopts = {'TCPKeepAlive':'no', 'StrictHostKeyChecking':'no', 'BatchMode':'no', 'CheckHostIP':'no', 'PasswordAuthentication':'no', 'PubkeyAuthentication':'yes', 'RequestTTY':'force'}
        self.sshfsopts = {'TCPKeepAlive':'no', 'StrictHostKeyChecking':'no', 'BatchMode':'no', 'CheckHostIP':'no', 'PasswordAuthentication':'no', 'PubkeyAuthentication':'yes'}
        for i in opts.keys():
            self.opts[i.replace('_', '-')] = opts[i]
        RemoteNemu.set[name] = self
        NemuNameSpace.set[name] = self
        printok()

    def running(self):
        '''Indicates if SSH is running'''
        return not self.proc == None and (self.proc.pid in NemuProc.procl)

    def is_sshfs(self):
        '''Indicates if SSHFS is running'''
        return not self.procfs == None and (self.procfs.pid in NemuProc.procl)

    def qstr_opt(self, opts):
        '''SSH options string'''
        com = ''
        keys = sorted(opts.keys())
        for i in keys:
            com += ' -o ' + str(i)
            if not opts[i] == None:
                com += '=' + str(opts[i])
        return com

    def ssh_qstr(self):
        '''SSH string'''
        opts = dict()
        opts.update(self.sshopts)
        opts.update(self.opts)
        return 'ssh -Y ' + self.qstr_opt(opts) + ' -p ' + str(self.port) + ' ' + self.user + '@' + self.host

    def sshfs_qstr(self):
        '''SSHFS'''
        opts = dict()
        opts.update(self.sshfsopts)
        opts.update(self.opts)
        com = 'sshfs -f -o follow_symlinks' + self.qstr_opt(opts) + ' -p ' + str(self.port) + ' ' + self.user + '@' + self.host + ':'
        self.sshsession = self.rsend('print NemuVar.workspace + "/" + NemuVar.session')
        com += self.sshsession[0].rstrip('\n\r') + ' ' + self.sshfs
        return com
        
    def open_sshfs(self):
        '''Starts SSHFS'''
        printc('Openning a remote directory for ' + str(self.name), 'cyan')
        if not self.running():
            raise NemuError(str(self.name) + ' is not running')
        if self.is_sshfs():
            printc(str(self.name) + ' is already mounted', 'magenta')
            return
        self.procfs = popenproc(self.sshfs_qstr())
        if not self.running():
            self.sshsession = None
            raise NemuError('Cannot open a remote directory for ' + str(self.name))

    def close_sshfs(self):
        '''Stops SSHFS'''
        if self.is_sshfs():
            printc('Closing the remote directory for ' + str(self.name), 'cyan')
            actproc('fusermount -u ' + self.sshfs)
            self.sshsession = None

    def start(self):
        '''Starts SSH'''
        if self.running():
            return
        printc('Launching the remote connection ' + str(self.name) , 'blue')
        self.proc = popenproc(self.ssh_qstr())
        if not self.running():
            raise NemuError('Cannot establish Remote connection with ' + str(self.user) + '@' + str(self.host) + ':' + str(self.port))
        printc('Connected to ' + str(self.user) + '@' +  str(self.host) + ':' + str(self.port), 'cyan')
        self.ssh_init()

    def stop(self):
        '''Starts SSH'''
        if self.running():
            pid = self.proc.pid
            self.close_sshfs()
            self._send('exit()')
            killprocl(pid)

    def delete(self):
        '''Deletes the remote connection'''
        if self.running():
            self.send('DelNemu()')
        self.stop()
        if os.path.isdir(self.vard):
            shutil.rmtree(self.vard, True)
        del RemoteNemu.set[self.name]
        del NemuNameSpace.set[self.name]
        printc('Remote connection ' + str(self.name) + ' is deleted', 'cyan')

    def ssh_init(self):
        '''Init string to launch NEmu'''
        self.rsend(self.launch)
        for line in self.buff:
            self.send(line)
        self.buff = list()

    def _send(self, line):
        '''Base socket sending'''
        if self.running():
            try:
                self.proc.stdin.write(str(line) + "\n")
            except (AttributeError, IOError, ValueError):
                printc("Cannot communicate with " + str(self.name), 'magenta')
                pass
        else:
            self.buff.append(line)

    def _recv(self):
        '''Base socket receving'''
        if self.running():
            try:
                return self.proc.stdout.readline().rstrip('\n')
            except (AttributeError, IOError, ValueError):
                printc("Cannot communicate with " + str(self.name), 'magenta')
                pass
        else:
            return NemuVar.rflag

    def send(self, line):
        '''Socket sending and printing result'''
        self._send(line)
        printlog(str(line).rstrip('\n'), logf=self.infile)
        self._recv()
        while True:
            com = self._recv()
            if not com:
                continue
            if NemuVar.rflag == com.rstrip("\n\r"):
                return
            else:
                sys.stdout.write(com + "\n")
                printlog(com, logf=self.logfile)

    def rsend(self, line):
        '''Socket sending and returing result'''
        self._send(line)
        #printlog(str(line).rstrip('\n'), logf=self.infile)
        self._recv()
        ret = []
        while True:
            com = self._recv()
            if not com:
                continue
            if NemuVar.rflag == com.rstrip("\n\r"):
                return ret
            else:
                ret.append(com)
                #printlog(com, logf=self.logfile)

    def fload(self, script):
        '''Sends a config file through SSH and execs it'''
        try:
            fd = open(script)
        except IOError:
            raise NemuError('Cannot open ' + str(script))
        conf = fd.readlines()
        fd.close()
        com = "import tempfile"
        self.rsend(com)
        com = "import os"
        self.rsend(com)
        com = "__rfile = tempfile.mktemp(prefix='__remote_file', dir=NemuVar.rootd)"
        self.rsend(com)
        com = "__rfd = file(__rfile, mode='w')"
        self.rsend(com)
        for i in conf:
            com = '__rfd.write(\'' + i.rstrip("\r\n").lstrip("\r").replace("'", '"') + '\')'
            self.rsend(com)
            com = '__rfd.write("\\n")'
            self.rsend(com)
        com = "__rfd.close()"
        self.rsend(com)
        com = "ImportNemu(__rfile)"
        self.rsend(com)
        com = "os.unlink(__rfile)"
        self.rsend(com)


    def interact(self):
        '''Launches interact mode'''
        oldtty = termios.tcgetattr(sys.stdin)
        self.rsend('import readline')
        self.rsend('import rlcompleter')
        self.rsend('readline.parse_and_bind("tab: complete")')
        self.rsend('readline.clear_history()')
        self._send('sys.argv.remove("--remoted")')
        self._recv()
        try:
            tty.setraw(sys.stdin.fileno())
            tty.setcbreak(sys.stdin.fileno())
            while True:
                if not self.proc:
                    break
                r, w, e = select.select([self.proc.stdout, sys.stdin], [], [])
                if not self.proc:
                    break
                if self.proc.stdout in r:
                    x = self.proc.stdout.read(1)
                    if len(x) == 0:
                        break
                    sys.stdout.write(x)
                    sys.stdout.flush()
                if sys.stdin in r:
                    x = sys.stdin.read(1)
                    if len(x) == 0:#or x == "\x1b":
                        break
                    self.proc.stdin.write(x)
        finally:
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, oldtty)
            self._send('del readline')
            self._recv()
            self._send('del rlcompleter')
            self._recv()
            self._send('sys.argv.append("--remoted")')
            self._recv()
            print('')

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ',user=' + printstr(self.user)
        com += ',host=' + printstr(self.host)
        com += ',port=' + printstr(self.port)
        com += ',path=' + printstr(self.path)
        com += ',**' + printstr(self.opts)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


def StartRemoteNemu(*names):
    '''StartRemoteNemu(*names)
    - Def -
        Inits NEmu remote connections through SSH.
    - Args -
        . names [list of str] --> names of remote connections (Default: starts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNemu.set.keys())
    for i in hlist:
        try:
            rcon = RemoteNemu.set[i]
            rcon.start()
        except KeyError:
            raise NemuError('Cannot find the Remote connection ' + str(i))

def MountRemoteNemu(*names):
    '''MountRemoteNemu(*names)
    - Def -
        Inits NEmu remote mounts through SSHFS.
    - Args -
        . names [list of str] --> names of remote connections (Default: mounts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNemu.set.keys())
    for i in hlist:
        try:
            rcon = RemoteNemu.set[i]
            rcon.open_sshfs()
        except KeyError:
            raise NemuError('Cannot find the Remote connection ' + str(i))

def ComRemoteNemu(name, *coms):
    '''ComRemoteNemu(name, *coms)
    - Def -
        Sends NEmu commands to a remote connection through SSH.
    - Args -
        . name [str] --> name of the remote connection
        . coms [list of str] --> list of commands
    '''
    try:
        rcon = RemoteNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the Remote connection ' + str(name))
    for com in coms:
        rcon.send(str(com))

def FileRemoteNemu(name, *files):
    '''FileRemoteNemu(name, *files)
    - Def -
        Sends NEmu script files to a remote connection through SSH.
    - Args -
        . name [str] --> name of the remote connection
        . files [list of str] --> list of script files
    '''
    try:
        rcon = RemoteNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the Remote connection ' + str(name))
    for f in files:
        rcon.fload(f)
        
def ShellRemoteNemu(name):
    '''ShellRemoteNemu(name)
    - Def -
        Opens a shell for a NEmu remote connection through SSH (ESC to exit the shell and return back to NEmu manager).
    - Args -
        . name [str] --> name of the remote connection
    '''
    try:
        rcon = RemoteNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the Remote connection ' + str(name))
    rcon.interact()


def StopRemoteNemu(*names):
    '''StopRemoteNemu(*names)
    - Def -
        Closes NEmu remote connections through SSH.
    - Args -
        . names [list of str] --> names of remote connections (Default: halts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNemu.set.keys())
    for i in hlist:
        try:
            rcon = RemoteNemu.set[i]
            rcon.stop()
        except KeyError:
            raise NemuError('Cannot find the Remote connection ' + str(i))

def UmountRemoteNemu(*names):
    '''UmountRemoteNemu(*names)
    - Def -
        Closes NEmu remote mounts through SSHFS.
    - Args -
        . names [list of str] --> names of remote connections (Default: unmounts all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNemu.set.keys())
    for i in hlist:
        try:
            rcon = RemoteNemu.set[i]
            rcon.close_sshfs()
        except KeyError:
            raise NemuError('Cannot find the Remote connection ' + str(i))

def DelRemoteNemu(*names):
    '''DelRemoteNemu(*names)
    - Def -
        Deletes NEmu remote connections through SSH.
    - Args -
        . names [list str] --> names of remote connections (Default: deletes all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(RemoteNemu.set.keys())
    for i in hlist:
        try:
            rcon = RemoteNemu.set[i]
            rcon.delete()
        except KeyError:
            raise NemuError('Cannot find the Remote connection ' + str(i))

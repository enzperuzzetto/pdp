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

'''This module manages system processes'''

import time
import os
import subprocess
import signal
import shlex

from nemu.error import NemuError
from nemu.msg import (printc, printlog)
from nemu.name import NemuNameSpace

class NemuProc():
    '''Nemu process class'''
    procl = list()
    def __init__(self):
        pass

def delpid(pid):
    '''Deletes a process PID from procl'''
    pname = pid[0]
    for name in NemuNameSpace.set:
        try:
            if NemuNameSpace.set[name].proc.pid == pid[0]:
                pname = name
                if NemuNameSpace.set[name].proc.nemufd:
                    NemuNameSpace.set[name].proc.nemufd.close()
                NemuNameSpace.set[name].proc = None
                break
        except AttributeError:
            pass
    if not (pid[1] == 0 or pid[1] == signal.SIGINT or pid[1] == signal.SIGTERM or pid[1] == signal.SIGKILL):
        printc(str(pname) + ' seems to die badly with dead code ' + str(pid[1]), 'magenta')
    elif not pname == pid[0]:
        printc(str(pname) + ' is down', 'cyan')
    if pid[0] in NemuProc.procl:
        NemuProc.procl.remove(pid[0])
        
def actproc(com):
    '''Launches a process in foreground.
    Returns the process return code.'''
    proc = shlex.split(com)
    printc('Launching ' + proc[0], 'cyan')
    p = subprocess.Popen(proc, stdin=subprocess.PIPE,  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in p.stdout:
        printlog(line.rstrip("\r\n").lstrip("\r"), dtime=False)
    try:
        p = p.wait()
        if p != 0:
            raise NemuError('Exec error of ' + proc[0] + ' with error code ' + str(p))
    except OSError:
        pass
    printc('Process ' + str(proc[0]) + ' is down', 'cyan')
    return p

def popenproc(com, logfile=None):
    '''Launches a process in background and pipes for I/O.
    Returns the subprocess object.'''
    proc = shlex.split(com)
    printc('Launching ' + proc[0], 'cyan')
    p = None
    fd = None
    if logfile:
        fd = open(logfile, 'a+')
    if fd:
        p = subprocess.Popen(proc, stdin=subprocess.PIPE,  stdout=fd, stderr=subprocess.STDOUT)
    else:
        p = subprocess.Popen(proc, stdin=subprocess.PIPE,  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if p.pid < 0:
        raise NemuError('Spawn error of ' + str(proc[0]))
    #printc('Process ' + str(p.pid) + ' launched', 'cyan')
    NemuProc.procl.append(p.pid)
    p.nemufd = fd
    return p

def waitprocl(*pids):
    '''Waiting for managed process end of life and cleans them from proc-list.'''
    this = list()
    if len(pids) == 0:
        this.extend(NemuProc.procl)
    else:
        this.extend(pids)
    while len(this) > 0:
        for i in this:
            if not i in NemuProc.procl:
                this.remove(i)
        time.sleep(0.1)

def killprocl(*pids):
    '''Kills some process from a proc-list.'''
    this = list()
    if len(pids) == 0:
        this.extend(NemuProc.procl)
    else:
        this.extend(pids)        
    while len(this) > 0:
        pid = this.pop()
        if NemuProc.procl.count(pid) == 0:
            printc('Process ' + str(pid) + ' does not seem to belong to the current session', 'magenta')
            continue
        try:
            #printc('Killing ' + str(pid), 'cyan')
            if pid in NemuProc.procl:
                os.kill(pid, signal.SIGINT)
            if pid in NemuProc.procl:
                os.kill(pid, signal.SIGTERM)
            if pid in NemuProc.procl:
                os.kill(pid, signal.SIGKILL)
            waitprocl(pid)
        except OSError:
            printc('Process ' + str(pid) + ' does not seem to be alive', 'magenta')

def ComProcNemu(name, *coms):
    '''ComProcNemu(name, *coms)
    - Def -
        Sends internal commands to a NEmu sub-process.
    - Args -
        . name [str] --> name of the processus
        . coms [list of str] --> list of commands
    '''
    try:
        vobj = NemuNameSpace.set[name]
    except KeyError:
        raise NemuError('Cannot find ' + str(name))
    for com in coms:
        printc('Sending \'' + str(com) + '\' to ' + str(name), 'cyan')
        vobj.send(str(com))

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

'''This module manages mobility'''

import os
import sys
import signal
import shutil
import threading
import socket
import select
import traceback

from nemu.msg import (printc, printstr, printok, printlog, printerr)
from nemu.proc import (actproc,popenproc)
from nemu.error import NemuError
from nemu.var import NemuVar
from nemu.name import NemuNameSpace
from nemu.path import (mpath, symlink)
from nemu.proc import (NemuProc, waitprocl, killprocl)
from nemu.join import (Join, Unjoin)
from nemu.iface import (ConfIface, UnconfIface)
from nemu.nemo import NemoParser

class _nemo_thread(threading.Thread):
    '''Nemo thread'''
    def __init__(self, mob):
        threading.Thread.__init__(self)
        self.end = True
        self.daemon = True
        self.mob = mob
        self._sock = None
        self.sock = None
    def init(self):
        self._sock = self.mob.subserver()
        self._sock = self._sock[0]
        self.sock = self._sock.makefile('r')
        self.mob.subrun()
    def run(self):
        self.end = False
        self.init()
        while not self.end:
            if not self._sock or not self.sock:
                return
            try:
                data = self.sock.readline()
            except IOError,AttributeError:
                continue
            self.mob.subexec(data)
    def stop(self):
        self.end = True
        try:
            self.sock.close()
            self._sock.close()
            self.mob.sock.close()
            self.mob.sock = None
            self.mob.thread = None
        except TypeError:
            pass
        
class MobNemu():
    '''MobNemu(name, nodes, clean, port)
    - Def -
        Mobility object.
    - Args -
        . name [str] --> name of the mobility object
        . nodes [list of str] --> list of VairLinks, i.e. list of mobile wireless devices
        . clean [bool] --> clean/unjoin all wireless links at the scheduler death (Default: False)
        . port [int] --> connection port (Default: autoset)
    '''
    set = dict()
    def __init__(self, name, nodes=list(), clean=False, port=None):
        if str(name) in NemuNameSpace.set:
            raise NemuError('Name ' + str(name) + ' already exists')
        if str(name) in MobNemu.set:
            raise NemuError('Mobility object ' + str(name) + ' already exists')
        printc('Setting up the mobility object ' + str(name), 'blue')
        self.name = name
        self.thread = None
        self.proc = None
        self.sock = None
        self.clean = clean
        self.arch = NemuVar.arch
        self.exe = mpath(NemuVar.nemurcd, NemuVar.nemo, NemuVar.nemo + '.' + str(self.arch))
        self.vard = mpath(NemuVar.mobd, str(self.name))
        self.logfile = mpath(self.vard, NemuVar.outlog)
        self.infile = mpath(self.vard, NemuVar.inlog)
        self.mobmf = mpath(self.vard, str(self.name) + '.mob')
        self.mobcf = mpath(self.vard, str(self.name) + '.con')
        if not os.path.isdir(self.vard):
            os.mkdir(self.vard)
        infd = open(self.infile, 'a+')
        infd.close()
        if not port:
            self.port = NemuVar.tport
            while NemuVar.tport in NemuVar.tports:
                NemuVar.tport += 1
                self.port = NemuVar.tport
        else:
            self.port = port
        if self.port in NemuVar.tports:
            printc(str(self.port) + " is already in used", "magenta")
        NemuVar.tports.append(self.port)
        self.addr = NemuVar.taddr
        self.nodes = list()
        self.nodes.extend(nodes)
        self.parser = NemoParser(self.nodes)
        MobNemu.set[self.name] = self
        NemuNameSpace.set[self.name] = self
        printok()
            
    def mobgen(self, width, height, time, events, speed, accel, step, src, type='nemo'):
        if not self.subrunning():
            if src:
                printc('Generating the connectivity scenario of ' + str(self.name) + ' using ' + str(src) + ' as a ' + str(type) + ' mobility scenario', 'blue')
                if not type == 'nemo' and not type == 'ns2':
                    raise NemuError('Mobility scenario type ' + str(type) + ' is unknown')
                shutil.copyfile(src, self.mobmf)
                self.substart()
                printc('Importing the mobility model of ' + str(self.name), 'cyan')
                self.send('map gen 1 ' + str(width) + ' ' + str(height) + ' 1 1')
                self.send('mob load ' + str(type) + ' ' + str(self.mobmf))
            else:
                printc('Generating the connectivity scenario of ' + str(self.name), 'blue')
                self.substart()
                printc('Generating the mobility model of ' + str(self.name), 'cyan')
                self.send('map gen 1 ' + str(width) + ' ' + str(height) + ' 1 1')
                self.send('mob gen ' + str(time) + ' ' + str(len(self.nodes)) + ' ' + str(events) + ' ' + str(speed) + ' ' + str(accel))
            printc('Generating the connectity model of ' + str(self.name), 'cyan')
            self.send('mob save nemo ' + str(self.mobmf))
            self.send('mob proc ' + str(step))
            self.send('cnn save ' + str(self.mobcf))
            self.send('exit')
            waitprocl(self.proc.pid)
            printok()

    def congen(self, time, events, bw):
        if not self.subrunning():
            printc('Generating the connectivity scenario of ' + str(self.name), 'blue')
            self.substart()
            self.send('cnn gen ' + str(time) + ' ' + str(len(self.nodes)) + ' ' + str(events) + ' ' + str(bw))
            self.send('cnn save ' + str(self.mobcf))
            self.send('exit')
            waitprocl(self.proc.pid)
            printok()

    def conimport(self, src):
        if not self.subrunning():
            printc('Importing the connectivity scenario ' + str(src) + ' for ' + str(self.name), 'blue')
            shutil.copyfile(src, self.mobcf)
            self.substart()
            self.send('cnn load ' + str(src))
            self.send('exit')
            waitprocl(self.proc.pid)
        printok()

    def conexport(self, dest=None):
        if not os.path.isfile(self.mobcf):
            raise NemuError(str(self.name) + ' has to be configure first')
        if dest:
            printc('Exporting the connectvity scenario of ' + str(self.name) + ' to ' + str(dest), 'blue')
            shutil.copyfile(self.mobcf, dest)
            printok()
        else:
            fd = open(self.mobcf, 'r')
            for line in fd.readlines():
                sys.stdout.write(line)
            fd.close()

    def mobexport(self, dest=None):
        if not os.path.isfile(self.mobmf):
            raise NemuError(str(self.name) + ' has to be configure first')
        if dest:
            printc('Exporting the mobility scenario of ' + str(self.name) + ' to ' + str(dest), 'blue')
            shutil.copyfile(self.mobmf, dest)
            printok()
        else:
            fd = open(self.mobmf, 'r')
            for line in fd.readlines():
                sys.stdout.write(line)
            fd.close()
        
    def subconnect(self):
        self.send('conn e tcp * * ' + str(self.addr) + ' ' + str(self.port))

    def subconfig(self):
        self.send('cnn load ' + str(self.mobcf))

    def subrun(self):
        self.send('cnn start')

    def subserver(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((self.addr, self.port))
        self.sock.listen(1)
        return self.sock.accept()
                    
    def substop(self):
        if self.thread:
            self.thread.stop()

    def substart(self):
        if not self.subrunning():
            self.proc = popenproc(self.qstr(), logfile=self.logfile)
        
    def subexec(self, data):
        ret = self.parser.parse(data)
        _quiet = NemuVar.quiet
        for line in ret:
            try:
                if line.startswith('self'):
                    exec(line)
                else:
                    _quiet = NemuVar.quiet
                    NemuVar.quiet = True
                    exec(line) in globals()
                    NemuVar.quiet = _quiet
            except NemuError as e:
                trace = traceback.extract_tb(sys.exc_traceback)
                pytrace = "\n" + reduce(lambda x, y:  x + y, traceback.format_list(trace))
                uline = trace[-1][1]
                ufile = trace[-1][0]
                if ufile == '<string>':
                    ufile = 'nemu'
                else:
                    ufile = os.path.basename(ufile)
                com = 'Runtime error [' + ufile  + ':'  + str(uline)  + '] : ' + str(e) + '.'
                NemuVar.quiet = _quiet
                printerr(com)
                printlog(pytrace, dtime=False)
                self.stop()
            except Exception as e:
                if isinstance(e, EOFError):
                    pass
                trace = traceback.extract_tb(sys.exc_traceback)
                pytrace = "\n" + reduce(lambda x, y: x + y, traceback.format_list(trace))
                uline = trace[-1][1]
                ufile = trace[-1][0]
                if ufile == '<string>':
                    ufile = 'nemu'
                else:
                    ufile = os.path.basename(ufile)
                com = 'Fatal error [' + ufile  + ':'  + str(uline)  + '] : ' + str(e) + '.'
                NemuVar.quiet = _quiet
                printerr(com)
                printlog(pytrace, dtime=False)
                self.stop()

    def freeze(self):
        if self.running():
            printc('Freezing the real time mobility scheduler of ' + str(self.name), 'cyan')
            os.kill(self.proc.pid, signal.SIGTSTP)
    
    def resume(self):
        if self.running():
            printc('Resuming the real time mobility scheduler of ' + str(self.name), 'cyan')
            os.kill(self.proc.pid, signal.SIGCONT)
        
    def subrunning(self):
        '''Indicates if the vlink is active'''
        return not self.proc == None and (self.proc.pid in NemuProc.procl)

    def running(self):
        '''Indicates if the vlink is active'''
        return self.subrunning() and self.thread

    def send(self, line):
        '''Write in the nemo stdin'''
        if self.subrunning():
            try:
                self.proc.stdin.write(str(line) + "\n")
                printlog(str(line).rstrip('\n'), logf=self.infile)
            except (AttributeError, IOError, ValueError):
                printc("Cannot communicate with " + str(NemuVar.nemo), 'magenta')
                pass

    def start(self):
        '''Starts nemo'''
        if self.running():
            return
        printc('Launching the real time mobility scheduler of ' + str(self.name) , 'blue')
        if not os.path.isfile(self.mobcf):
            raise NemuError(str(self.name) + ' has to be configured or generated first')
        self.substart()
        self.thread = _nemo_thread(self)
        self.thread.start()
        self.subconfig()
        self.subconnect()
        #self.subrun()
        printok()

    def stop(self):
        '''Stops nemo'''
        if self.subrunning():
            printc('Stopping the real time mobility scheduler of ' + str(self.name) , 'cyan')
            self.send("disc e")
            self.send("exit")
            self.substop()
            killprocl(self.proc.pid)
            if self.clean:
                printc('Cleaning wireless links of ' + str(self.name) , 'cyan')
                for wic in self.nodes:
                    Unjoin(wic)

    def delete(self):
        '''Deletes nemo'''
        printc('Deleting the mobility object ' + str(self.name) , 'blue')
        self.stop()
        if os.path.isdir(self.vard):
            shutil.rmtree(self.vard, True)
        del MobNemu.set[self.name]
        del NemuNameSpace.set[self.name]
        printok()

    def qstr(self):
        '''QEMU string'''
        return self.exe + ' -d ' + self.vard + '/'
            
    def __str__(self):
        com = self.__class__.__name__ +'('
        com += 'name=' + printstr(self.name)
        com += ',nodes=' + printstr(self.nodes)
        com += ',port=' + printstr(self.port)
        com += ')'
        return com

    def __repr__(self):
        return str(self)


def GenMobNemu(name, width=1000, height=1000, time=10, events=100, speed=1, accel=0, step=1, src=None, type='nemo'):
    '''GenMobNemu(name, width, height, time, events, speed, accel, step, src, type)
    - Def -
        Generate a connectivity scenario.
    - Args -
        . name [str] --> name of the desired mobility object
        . width [int] --> area width (Default: 1000)
        . height [int] --> area height (Default: 1000)
        . time [int] --> mobility scenario duration in seconds (Default: 10)
        . events [int] --> mobility events number (Default: 100)
        . speed [int] --> max node speed (Default: 1)
        . accel [int] --> max node acceleration (Default: 0)
        . step [float] --> real time scheduler timestamp (Default: 1)
        . src [str] --> use a mobility scenario stored in the file src instead of generating it (Default: None)
        . type [str] --> mobility scenario file type : 'nemo' or 'ns2' (Default: nemo)
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility scenario ' + str(name))
    mob.mobgen(width, height, time, events, speed, accel, step, src, type)


def ExportMobNemu(name, mobdest=None, condest=None):
    '''ExportMobNemu(name, dest)
    - Def -
        Exports mobility and connectivity scenarios.
    - Args -
        . name [str] --> name of the desired mobility object
        . mobdest [str] --> mobility scenario destination file (Default: None)
        . condest [str] --> connectivity scenario destination file (Default: None)
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.mobexport(mobdest)
    mob.conexport(condest)

def ImportMobNemu(name, src):
    '''ImportMobNemu(name, src)
    - Def -
        Imports a connectivity scenario.
    - Args -
        . name [str] --> name of the desired mobility object
        . src [str] --> source file
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.conimport(src)

def FreezeMobNemu(name):
    '''FreezeMobNemu(name)
    - Def -
        Freezes a mobility real time scheduler.
    - Args -
        . name [str] --> name of the desired mobility object
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.freeze()

def ResumeMobNemu(name):
    '''ResumeMobNemu(name)
    - Def -
        Resumes a mobility real time scheduler.
    - Args -
        . name [str] --> name of the desired mobility object
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.resume()

def StartMobNemu(name):
    '''StartMobNemu(name)
    - Def -
        Starts a mobility real time scheduler.
    - Args -
        . name [str] --> name of the desired mobility object
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.start()

def StopMobNemu(name):
    '''StopMobNemu(name)
    - Def -
        Stops a mobility real time scheduler.
    - Args -
        . name [str] --> name of the desired mobility object
    '''
    try:
        mob = MobNemu.set[name]
    except KeyError:
        raise NemuError('Cannot find the mobility object ' + str(name))
    mob.stop()

def DelMobNemu(*names):
    '''DelMobNemu(*names)
    - Def -
        Deletes some mobility objects.
    - Args -
        . names [list of str] --> names of the desired mobility objects (Default: deletes all)
    '''
    hlist = list(names)
    if len(hlist) == 0:
        hlist = sorted(MobNemu.set.keys())
    for i in hlist:
        try:
            mob = MobNemu.set[i]
        except KeyError:
            raise NemuError('Cannot find the mobility object ' + str(i))
        mob.delete()

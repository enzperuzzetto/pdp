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

'''This module contains the main NEmu loop'''

import sys
import os
import traceback
import time
import code
import threading

from nemu.var import NemuVar
from nemu.msg import (printerr, printlog)
from nemu.error import NemuError
from nemu.remote import (isremoted, RemoteNemu, StartRemoteNemu, FileRemoteNemu, ShellRemoteNemu, StopRemoteNemu, DelRemoteNemu)
from nemu.session import InitNemu
from nemu.delete import DelNemu
from nemu.proc import delpid
from nemu.hist import save_nemu_history

from nemu import *


class _process_loop(threading.Thread):
    '''Core process thread code'''
    status = False
    def __init__(self):
        if _process_loop.status:
            raise NemuError("Nemu init threads are already running")
        threading.Thread.__init__(self)
        self.end = True
        self.daemon = True
        _process_loop.status = True
    def run(self):
        self.end = False
        while not self.end:
            try:
                pid = os.waitpid(-1, 0)
                delpid(pid)
            except OSError:
                pass
            time.sleep(0.1)
    def stop(self):
        '''Stop thread flag'''
        self.end = True

def process_loop():
    '''Process thread start point'''
    thread = _process_loop()
    thread.start()

def remote_interactive_loop(conf, filename=None, interactive=True):
    '''Remote loop mode'''
    port = 22
    path = None
    user, host = conf.split('@')
    if ':' in host:
        if host.count(':') == 1:
            host, port = host.split(':')
        elif host.count(':') == 2:
            host, port, path = host.split(':')
        else:
            raise NemuError('Cannot parse ' + str(host))
    name = host
    InitNemu()
    RemoteNemu(name, user, host, int(port), path)
    StartRemoteNemu(name)
    if filename and os.path.isfile(filename):
        FileRemoteNemu(name, filename)
    if interactive:
        ShellRemoteNemu(name)
    StopRemoteNemu(name)
    DelRemoteNemu(name)
    DelNemu()

def main_loop(filename=None):
    '''Base loop mode'''
    fd = None
    errline = 0
    try:
        if filename:
            fd = open(filename, 'r')
            fd.close()
            execfile(filename, globals(), globals())
        else:
            rcon = code.InteractiveConsole(locals=globals())
            while True:
                try:
                    if isremoted():
                        print(NemuVar.rflag)
                    val = rcon.raw_input(sys.ps1)
                    save_nemu_history(val)
                    exec(val) in globals()
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
                    if filename and errline > 0:
                        com += ' (' + str(filename) + ':' + str(errline) + ')'
                    printerr(com)
                    printlog(pytrace, dtime=False)
                    pass
                except Exception as e:
                    if isinstance(e, EOFError):
                        print ''
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
                    if filename and errline > 0:
                        com += ' (' + str(filename) + ':' + str(errline) + ')'
                    printerr(com)
                    printlog(pytrace, dtime=False)
                    pass
                except KeyboardInterrupt as e:
                    print ''
                    com = 'Interrupted.'
                    printerr(com)
                    pass
                
    except NemuError as e:
        trace = traceback.extract_tb(sys.exc_traceback)
        pytrace = "\n" + reduce(lambda x, y: x + y, traceback.format_list(trace))
        uline = trace[-1][1]
        ufile = trace[-1][0]
        if ufile == '<string>':
            ufile = 'nemu'
        else:
            ufile = os.path.basename(ufile)
        com = 'Runtime error [' + ufile  + ':'  + str(uline)  + '] : ' + str(e) + '.'
        if filename and errline > 0:
            com += ' (' + str(filename) + ':' + str(errline) + ')'
        printerr(com)
        printlog(pytrace, dtime=False)
        pass
    except Exception as e:
        if isinstance(e, EOFError):
            print ''
            pass
        trace = traceback.extract_tb(sys.exc_traceback)
        pytrace = "\n" + reduce(lambda x, y:  x + y, traceback.format_list(trace))
        uline = trace[-1][1]
        ufile = trace[-1][0]
        if ufile == '<string>':
            ufile = 'nemu'
        else:
            ufile = os.path.basename(ufile)
        com = 'Fatal error [' + ufile  + ':'  + str(uline)  + '] : ' + str(e) + '.'
        if filename and errline > 0:
            com += ' (' + str(filename) + ':' + str(errline) + ')'
        printerr(com)
        printlog(pytrace, dtime=False)
        pass
    except KeyboardInterrupt as e:
        print ''
        com = 'Interrupted.'
        printerr(com)
        pass

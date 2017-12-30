#!/usr/bin/env python

# ---------------------------------
# -- NEmu : The Network Emulator --
# ---------------------------------

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

'''Init setup script'''

import os
import sys
import re
import subprocess
import platform
import traceback

class _log():
    '''Log file operations'''
    def __init__(self, name):
        self.name = name
        self.fd = open(name, 'w')
        print('Writing logs in ' + self.name)
    def write(self, data):
        '''Writes line'''
        self.fd.write(data)
    def close(self):
        '''Closes file'''
        self.fd.close()
    def delete(self):
        '''Deletes file'''
        os.unlink(self.name)

def _actproc(com, log):
    '''Launches a process'''
    proc = com.split()
    while proc.count('') > 0:
        proc.remove('')
    log.write(com + "\n")
    print('Launching ' + com)
    p = subprocess.Popen(proc, stdin=subprocess.PIPE,  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in p.stdout:
        log.write(line)
    log.write("\n")
    p = p.wait()
    if p != 0:
        print('Exec error of ' + proc[0] + ' with error code ' + str(p))
    print('Done')
    return p

def _sed(old, new, src, dst=None):
    '''String replace'''
    if not dst:
        dst = src
    print('Patching ' + dst)
    pattern = re.compile(old, re.MULTILINE)
    fd = open(src, "r")
    content = fd.read()
    fd.close()
    fd = open(dst, "w")
    fd.write(pattern.sub(new, content))
    fd.close()
    print('Done')
    
def main(log):
    '''Main init function'''
    try:
        _os = platform.system()
        _proc = platform.machine()
        arch = _os + '-' + _proc
        sl = 'so'
        mt = '-mt'
        import readline
        import rlcompleter
        _actproc('python --version', log)
        _actproc('qemu-system-x86_64 --version', log)
        _actproc('locate --version', log)
        _actproc('mksquashfs -version', log)
        _actproc('make --version', log)
        _actproc('g++ --version', log)
        _actproc('dot -V', log)
        _actproc('ssh -V', log)
        _actproc('sshfs --version', log)
        _actproc('vde_switch --version', log)
        if _os == 'Darwin':
            sl = 'dylib'
        _actproc('locate libcrypto.' + sl, log)
        _actproc('locate libssl.' + sl, log)
        _actproc('locate libpthread.' + sl, log)
        _actproc('locate libreadline.' + sl, log)
        _actproc('locate libvdeplug.' + sl, log)
        if not _actproc('locate libboost_system' + mt + '.' + sl, log) == 0:
            mt = ''
        _actproc('locate libboost_system' + mt + '.' + sl, log)
        _actproc('locate libboost_thread' + mt + '.' + sl, log)
        _actproc('locate libboost_chrono' + mt + '.' + sl, log)
        _actproc('locate libboost_regex'  + mt + '.' + sl, log)
        makefile = 'Makefile'
        makefile_root = 'rcd/tools/Makefile'
        makefile_vnd = 'rcd/vnd/Makefile'
        makefile_nemo = 'rcd/nemo/Makefile'
        _sed("^LD_LIBS(.*)$", "LD_LIBS= rt ssl crypto readline vdeplug boost_system" + mt + " boost_thread" + mt + " boost_chrono" + mt + " boost_regex" + mt, makefile_root, makefile_vnd)
        _sed("^LD_LIBS(.*)$", "LD_LIBS= rt ssl crypto readline vdeplug boost_system" + mt + " boost_thread" + mt + " boost_chrono" + mt + " boost_regex" + mt, makefile_root, makefile_nemo)
        _sed("^OUTFILE=(.*)$", "OUTFILE=vnd." + arch, makefile_vnd)
        _sed("^OUTFILE=(.*)$", "OUTFILE=nemo." + arch, makefile_nemo)
        if _os == 'Darwin':
            _sed("rt", "", makefile_vnd)
            _sed("rt", "", makefile_nemo)
        _actproc('make out --directory=rcd/vnd -f ' + makefile, log)
        _actproc('make clean-o --directory=rcd/vnd -f ' + makefile, log)
        _actproc('make out --directory=rcd/nemo -f ' + makefile, log)
        _actproc('make clean-o --directory=rcd/nemo -f ' + makefile, log)
        os.unlink(makefile_vnd)
        os.unlink(makefile_nemo)

    except Exception as e:
        print(e)
        if log:
            trace = traceback.extract_tb(sys.exc_traceback)
            pytrace = str(reduce(lambda x, y: x + y, traceback.format_list(trace))) + "\n"
            log.write(pytrace)

if __name__ == '__main__':
    log = _log('init.log')
    main(log)
    log.close()

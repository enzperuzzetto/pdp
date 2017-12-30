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

'''This module manages virtual links'''

from nemu.name import NemuNameSpace
from nemu.error import NemuError
from nemu.var import NemuVar
from nemu.msg import (printc, printok, printstr, printlog)
from nemu.proc import (popenproc, NemuProc, killprocl)
from nemu.path import mpath
from nemu.iface import VLic

import shutil
import os


class VLink():
    '''VLink(name, type, niface)
    - Def -
        A VLink represents a virtual connector between virtual machines.
    - Args -
        . name [str] --> VLink name
        . type [str] --> VLink type 
        . niface [int] --> number of interfaces
    '''
    set = dict()
    def __init__(self, name, type=None, niface=0):
        if str(name) in NemuNameSpace.set:
            raise NemuError('Name ' + str(name) + ' already exists')
        if str(name) in VLink.set:
            raise NemuError('Virtual Link ' + str(name) + ' already exists')
        self.name = str(name)
        self.arch = NemuVar.arch
        self.exe = mpath(NemuVar.nemurcd, NemuVar.vnd, NemuVar.vnd + '.' + str(self.arch))
        self.type = type
        self.proc = None
        self.vard = mpath(NemuVar.linkd, str(self.name))
        self.logfile = mpath(self.vard, NemuVar.outlog)
        self.infile = mpath(self.vard, NemuVar.inlog)
        if not os.path.isdir(self.vard):
            os.mkdir(self.vard)
        infd = open(self.infile, 'a')
        infd.close()
        self.ifaces = list()
        for i in range(niface):
            iface = VLic(str(i))
            iface.hostname = self.name
            iface.dumpfile = mpath(self.vard, str(self.name) + '.' + str(iface.id) + '.cap')
            self.ifaces.append(iface)
        VLink.set[self.name] = self
        NemuNameSpace.set[self.name] = self

    def findiface(self, id):
        '''Finds the iface called <id>'''
        for i in range(len(self.ifaces)):
            if str(self.ifaces[i].id) == str(id):
                return self.ifaces[i]
        raise NemuError(self.name + ' has not any interface ' + str(id))

    def isiface(self, id):
        '''Indicates if the iface called <id> exists'''
        for i in range(len(self.ifaces)):
            if str(self.ifaces[i].id) == str(id):
                return True
        return False

    def addiface(self, id=None):
        '''Adds a new iface called <id>'''
        if not id:
            id = len(self.ifaces)
        if self.isiface(id):
            raise NemuError(str(id) + " already exists")
        iface = VLic(id)
        iface.hostname = self.name
        iface.dumpfile = mpath(self.vard, str(self.name) + '.' + str(iface.id) + '.cap')
        self.ifaces.append(iface)
        self.addlink(iface.id)
        printc(str(self.name) + ' has a new interface ' + str(id), "cyan")

    def addlink(self, id):
        '''Proc adds the iface called <id>'''
        if self.running():
            self.send("add i" + str(id))

    def deliface(self, id):
        '''Deletes the iface called <id>'''
        iface = self.findiface(id)
        self.ifaces.remove(iface)
        self.dellink(id)
        printc(str(self.name) + ':' + str(id) + ' is now removed', "cyan")

    def dellink(self, id):
        '''Proc deletes the iface called <id>'''
        if self.running():
            self.send("disc e" + str(id))
            self.send("rem i" + str(id))
        
    def dumpiface(self, id):
        '''Starts dumping the iface called <id>'''
        iface = self.findiface(id)
        if not iface.dump:
            iface.dump = True
            self.dumplink(id)
            printc(str(self.name) + ':' + str(iface.id) +  ' now dumps traffic', 'cyan')
        else:
            printc(str(self.name) + ':' + str(iface.id) +  ' is already dumping', 'magenta')

    def dumplink(self, id):
        '''Proc starts dumping the iface called <id>'''
        iface = self.findiface(id)
        if self.running() and iface.dump:
            self.send("trace i" + str(iface.id) + " pcap " + str(iface.dumpfile))

    def undumpiface(self, id):
        '''Stops dumping the iface called <id>'''
        iface = self.findiface(id)
        if iface.dump:
            iface.dump = False
            self.undumplink(id)
            printc(str(self.name) + ':' + str(iface.id) +  ' stops to dump traffic', 'cyan')
        else:
            printc(str(self.name) + ':' + str(iface.id) +  ' is not dumping', 'magenta')

    def undumplink(self, id):
        '''Proc stops dumping the iface called <id>'''
        iface = self.findiface(id)
        if self.running():
            self.send("untrace i" + str(iface.id) + " pcap " + str(iface.dumpfile))

    def setiface(self, id, addr, port, type, proto, laddr, lport):
        '''Sets connection information of <id>'''
        if not proto:
            proto = 'tcp'
        if proto == 'tcp' or proto == 'udp':
            self.inetiface(id, addr, port, type, proto, laddr, lport)
        elif proto == 'unix':
            self.unixiface(id, path=addr, type=type)
        elif proto == 'vde':
            self.vdeiface(id, path=addr, type=type)
        elif proto == 'tap':
            self.tapiface(id, ifname=addr, type=type)
        elif proto == 'slirp':
            self.slirpiface(id, net=None, restrict=False, forwards=None, type=type)
        else:
            raise NemuError(str(proto) + ' is not a valid protocol')

    def inetiface(self, id, addr, port, type, proto, laddr, lport):
        '''Sets inet information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.set(addr, port, type, proto, laddr, lport)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with ' + str(iface.proto) + '-' + str(iface.type) + '-' + str(iface.addr) + ':' + str(iface.port) + '-' + str(iface.laddr) + ':' + str(iface.lport), 'cyan')

    def unixiface(self, id, path, type):
        '''Sets unix information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        if type == 'srv' and not path:
            path = mpath(self.vard, 'unix-' + str(iface.id))
        iface.unix(path, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with unix-' + str(iface.type) + '-' + str(iface.addr), 'cyan')
            
    def vdeiface(self, id, path, type):
        '''Sets vde information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.vde(path, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with vde-' + str(iface.type) + '-' + str(iface.addr), 'cyan')
            
    def tapiface(self, id, ifname, type):
        '''Sets tap information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.tap(ifname, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with tap-' + str(iface.type) + '-' + str(iface.addr), 'cyan')
        
    def slirpiface(self, id, net, restrict, forwards, type):
        '''Sets slirp information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.slirp(net, restrict, forwards, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with slirp-' + str(iface.net), 'cyan')
            
    def setlink(self, id):
        '''Proc sets connection information of <id>'''
        iface = self.findiface(id)
        if self.running() and iface.activ:
            if iface.proto == 'tcp':
                packet_type = "len"
                if iface.type == 'srv':
                    self.send("insr e" + str(id) + " " + packet_type + " tcp " + iface.addr + " " + str(iface.port) + " ? ? ")
                elif iface.type == 'cli':
                    self.send("incn e" + str(id) + " " + packet_type + " tcp * * " + iface.addr + " " + str(iface.port))
            elif iface.proto == 'udp':
                packet_type = "raw"
                if iface.type == 'srv' or iface.type == 'peer':
                    self.send("insr e" + str(id) + " " + packet_type + " udp " + iface.addr + " " + str(iface.port) + " " + iface.laddr + " " + str(iface.lport))
                elif iface.type == 'cli':
                    self.send("incn e" + str(id) + " " + packet_type + " udp " + iface.laddr + " " + str(iface.lport) + " " + iface.addr + " " + str(iface.port))
            elif iface.proto == 'mcast':
                packet_type = "raw"
                self.send("incn e" + str(id) + " " + packet_type + " udp " + iface.addr + " " + str(iface.port) + " " + iface.laddr + " " + str(iface.lport))
            elif iface.proto == 'unix':
                if iface.type == 'srv':
                    self.send("unsr e" + str(id) + " raw dgm " + str(iface.addr))
                elif iface.type == 'cli':
                    self.send("uncn e" + str(id) + " raw dgm " + str(iface.addr))
            elif iface.proto == 'vde':
                self.send("vde" + " e" + str(id) + " " + str(iface.addr))
            elif iface.proto == 'tap':
                self.send("tap" + " e" + str(id) + " " + str(iface.addr))
            else:
                raise NemuError(str(iface.proto) + ' is not supported by VLink')
            self.send("bind e" + str(id) + " i" + str(id))

    def unsetiface(self, id):
        '''Unsets connection information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            iface.unset()
            self.unsetlink(id)
            printc(str(self.name) + ':' + str(iface.id) + ' is now unset', 'cyan')
        
    def unsetlink(self, id):
        '''Proc unsets connection information of <id>'''
        iface = self.findiface(id)
        if self.running():
            self.send("unbind e" + str(id) + " i" + str(id))
            self.send("disc e" + str(id))

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Sets properties of <id>'''
        iface = self.findiface(id)
        iface.conf(direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
        self.conflink(id)
        if direction == "in" or direction == "all":
            com = str(self.name) + ':' + str(id) +  ' is now configured with ' + 'in:state=' + str(iface.instate) + ',vlan=' + str(iface.invli) 
            com += ',bandwidth=' + str(iface.inbw) + ',delay=' + str(iface.indl) + ',jitter=' + str(iface.injit)
            com += ',ber=' + str(iface.inber) + ',qsize=' + str(iface.inqs)
            printc(com, 'cyan')
        if direction == "out" or direction == "all":
            com = str(self.name) + ':' + str(id) +  ' is now configured with ' + 'out:state=' + str(iface.outstate) + ',vlan=' + str(iface.outvli) 
            com += ',bandwidth=' + str(iface.outbw) + ',delay=' + str(iface.outdl) + ',jitter=' + str(iface.outjit)
            com += ',ber=' + str(iface.outber) + ',qsize=' + str(iface.outqs)
            printc(com, 'cyan')
        
    def conflink(self, id):
        '''Proc sets properties of <id>'''
        iface = self.findiface(id)
        if self.running():
            if not iface.instate == None:
                self.send(str(iface.instate) + " i" + str(id) + " in")
            if not iface.invli == None:
                self.send("set i" + str(id) + " in vli " + str(iface.invli))
            if not iface.inbw == None:
                self.send("set i" + str(id) + " in bw " + str(iface.inbw))
            if not iface.indl == None:
                self.send("set i" + str(id) + " in dl " + str(iface.indl))
            if not iface.injit == None:
                self.send("set i" + str(id) + " in dv " + str(iface.injit))
            if not iface.inber == None:
                self.send("set i" + str(id) + " in ber " + str(iface.inber))
            if not iface.inqs == None:
                self.send("set i" + str(id) + " in qs " + str(iface.inqs))
            if not iface.outstate == None:
                self.send(str(iface.outstate) + " i" + str(id) + " out")
            if not iface.outvli == None:
                self.send("set i" + str(id) + " out vli " + str(iface.outvli))
            if not iface.outbw == None:
                self.send("set i" + str(id) + " out bw " + str(iface.outbw))
            if not iface.outdl == None:
                self.send("set i" + str(id) + " out dl " + str(iface.outdl))
            if not iface.outjit == None:
                self.send("set i" + str(id) + " out dv " + str(iface.outjit))
            if not iface.outber == None:
                self.send("set i" + str(id) + " out ber " + str(iface.outber))
            if not iface.outqs == None:
                self.send("set i" + str(id) + " out qs " + str(iface.outqs))

    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Unsets properties of <id>'''
        iface = self.findiface(id)
        iface.unconf(direction, state, vlan, bandwidth, delay, jitter, ber, qsize)
        self.unconflink(id)
        com = str(self.name) + ':' + str(id) + ' is now unconfigured for ' + str(direction) + ':'
        subcom = ''
        if state:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'state'
        if vlan:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'vlan'
        if bandwidth:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'bandwidth'
        if delay:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'delay'
        if jitter:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'jitter'
        if ber:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'ber'
        if qsize:
            if len(subcom) > 0 :
                subcom += ','
            subcom += 'qsize'
        printc(com + subcom, 'cyan')
        
    def unconflink(self, id):
        '''Proc unsets properties of <id>'''
        iface = self.findiface(id)
        if self.running():
            if iface.instate == None:
                self.send("up i" + str(id) + " in")
            if iface.invli == None:
                self.send("unset i" + str(id) + " in vli")
            if iface.inbw == None:
                self.send("unset i" + str(id) + " in bw")
            if iface.indl == None:
                self.send("unset i" + str(id) + " in dl")
            if iface.injit == None:
                self.send("unset i" + str(id) + " in dv")
            if iface.inber == None:
                self.send("unset i" + str(id) + " in ber")
            if iface.inqs == None:
                self.send("unset i" + str(id) + " in qs")
            if iface.outstate == None:
                self.send("up i" + str(id) + " out")
            if iface.outvli == None:
                self.send("unset i" + str(id) + " out vli")
            if iface.outbw == None:
                self.send("unset i" + str(id) + " out bw")
            if iface.outdl == None:
                self.send("unset i" + str(id) + " out dl")
            if iface.outjit == None:
                self.send("unset i" + str(id) + " out dv")
            if iface.outber == None:
                self.send("unset i" + str(id) + " out ber")
            if iface.outqs == None:
                self.send("unset i" + str(id) + " out qs")

    def start(self):
        '''Starts the vlink'''
        if self.running():
            return
        printc('Launching the VLink ' + str(self.name) , 'blue')
        self.proc = popenproc(self.qstr(), logfile=self.logfile)
        if self.running():
            for i in self.ifaces:
                self.addlink(i.id)
                self.dumplink(i.id)
                self.conflink(i.id)
                if i.type == 'srv' or i.type == 'peer':
                    self.setlink(i.id)
            self.setvlink()
        printok()

    def stop(self):
        '''Stops the vlink'''
        if self.running():
            printc('Stopping the VLink ' + str(self.name) , 'cyan')
            pid = self.proc.pid
            self.unsetvlink()
            for i in self.ifaces:
                self.unsetlink(i.id)
            self.send("exit")
            killprocl(pid)

    def reboot(self):
        '''Restarts the vlink'''
        if self.running():
            printc('Rebooting the VLink ' + str(self.name) , 'cyan')
            self.stop()
            self.start()

    def send(self, line):
        '''Write in the vlink stdin'''
        if self.running():
            try:
                self.proc.stdin.write(str(line) + "\n")
                printlog(str(line).rstrip('\n'), logf=self.infile)
            except (AttributeError, IOError, ValueError):
                printc("Cannot communicate with " + str(self.name), 'magenta')
                pass

    def running(self):
        '''Indicates if the vlink is active'''
        return not self.proc == None and (self.proc.pid in NemuProc.procl)

    def setvlink(self):
        '''Sets the global parameters of the vlink'''
        return

    def unsetvlink(self):
        '''Unsets the global parameters of the vlink'''
        return

    def delete(self):
        '''Deletes the vlink'''
        self.stop()
        if os.path.isdir(self.vard):
            shutil.rmtree(self.vard, True)
        for i in self.ifaces:
            i.delete()
        del VLink.set[self.name]
        del NemuNameSpace.set[self.name]

    def qstr(self):
        '''QEMU string'''
        return self.exe + ' -n ' + str(self.name) + ' -d ' + self.vard + '/'

    def gstr(self):
        '''Graph description'''
        com = str(self.name) + ' [shape=ellipse,peripheries=2,color='
        if self.running():
            com += 'green'
        else:
            com += 'lightgray'
        com += ',style=filled,label="' + str(self.__class__.__name__) + '['+ str(self.name) + ']"];'
        return com
            
    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ',type=' + printstr(self.type)
        com += ',niface=' + printstr(len(self.ifaces))
        com += ')'
        return com

    def __repr__(self):
        return str(self)

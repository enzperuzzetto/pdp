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

'''This module manages virtual nodes'''

from nemu.name import NemuNameSpace
from nemu.var import NemuVar
from nemu.error import NemuError
from nemu.msg import (printc, printok, printstr, printlog)
from nemu.proc import (popenproc, killprocl, NemuProc)
from nemu.path import mpath
from nemu.iface import VNic

import shutil
import os

class VNode:
    '''Common Node core definition'''
    set = dict()
    def __init__(self, name, hds=list(), nics=list(), qemu=None, **opts):
        if str(name) in NemuNameSpace.set:
            raise NemuError(str(name) + ' already exists in the current namespace')
        if str(name) in VNode.set:
            raise NemuError('VNode ' + str(name) + ' already exists')
        self.name = str(name)
        self.hds = list()
        self.ifaces = list()
        self.proc = None
        self.type = None
        if not qemu:
            self.qemu = NemuVar.qemu
        else:
            self.qemu = qemu
        self.opts = dict()
        for i in opts.keys():
            self.opts[i.replace('_', '-')] = opts[i]
        self.vard = mpath(NemuVar.noded, str(self.name))
        self.logfile = mpath(self.vard, NemuVar.outlog)
        self.infile = mpath(self.vard, NemuVar.inlog)
        if not os.path.isdir(self.vard):
            os.mkdir(self.vard)
        infd = open(self.infile, 'a')
        infd.close()
        for i in nics:
            i.id = len(self.ifaces)
            i.hostname = self.name
            i.dumpfile = mpath(self.vard, str(self.name) + '.' + str(i.id) + '.' + i.hw.replace(':', '-') + '.cap')
            self.ifaces.append(i)
        for i in hds:
            self.hds.append(i)
        VNode.set[name] = self
        NemuNameSpace.set[name] = self

    def findiface(self, id):
        '''Finds the iface called <id>'''
        if int(id) >= len(self.ifaces):
            raise NemuError(self.name + ' has not any interface ' + str(id))
        return self.ifaces[int(id)]
    
    def isiface(self, id):
        '''Finds the iface called <id>'''
        if int(id) >= len(self.ifaces):
            return False
        return True

    def addiface(self, iface=None):
        '''Adds a new iface called <id>'''
        if not iface:
            iface = VNic()
        iface.id = len(self.ifaces)
        iface.hostname = self.name
        iface.dumpfile = mpath(self.vard, str(self.name) + '.' + str(iface.id) + '.' + iface.hw.replace(':', '-') + '.cap')
        self.ifaces.append(iface)
        self.addlink(iface.id)
        printc(str(self.name) + ' has a new interface ' + str(iface.hw), "cyan")
        
    def addlink(self, id):
        '''Proc adds the iface called <id>'''
        iface = self.findiface(id)
        if self.running():
            com = 'device_add ' + str(iface.model) + ',vlan=' + str(id) + ',mac=' + str(iface.hw)
            #com = 'pci_add auto nic ' + 'vlan=' + str(iface.id) + ",macaddr=" + str(iface.hw)
            #if iface.model:
            #    com += ',model=' +  str(iface.model)
            self.send(com)

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
            self.send("host_net_add dump vlan=" + str(iface.id) + ",name=dump." + str(iface.id) + ",file=" + str(iface.dumpfile))

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
            self.send("host_net_remove " + str(iface.id) + " dump." + str(iface.id))

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

    def slirpiface(self, id, net, restrict, forwards, type):
        '''Sets slirp information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.slirp(net, restrict, forwards, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with slirp-' + str(iface.net), 'cyan')
        
    def vdeiface(self, id, path, type):
        '''Sets vde information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.vde(path, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with vde-' + str(iface.type) + '-' + str(iface.addr), 'cyan')

    def unixiface(self, id, path, type):
        '''Sets unix information of <id>'''
        raise NemuError('UNIX is not available for VNode')

    def tapiface(self, id, ifname, type):
        '''Sets tap information of <id>'''
        iface = self.findiface(id)
        if iface.activ:
            raise NemuError(str(self.name) + ':' + str(id) + ' is already set')
        iface.tap(ifname, type)
        self.setlink(id)
        printc(str(self.name) + ':' + str(id) + ' is now set with tap-' + str(iface.type) + '-' + str(iface.addr), 'cyan')

    def setlink(self, id):
        '''Proc sets connection information of <id>'''
        iface = self.findiface(id)
        if self.running() and iface.activ:
            com = "host_net_add socket " + "name=socket." + str(iface.id) + ",vlan=" + str(iface.id) + ","
            if iface.proto == 'mcast':
                com += "mcast" + "=" + str(iface.addr) + ":" + str(iface.port)
                if iface.laddr:
                    com += ",localaddr="  + str(iface.laddr)
            elif iface.proto == 'udp':
                if iface.type == 'srv' or iface.type == 'peer':
                    com += "udp" + "=" + str(iface.laddr) + ":" + str(iface.lport) + ",localaddr=" + str(iface.addr) + ':' +  str(iface.port)
                elif iface.type == 'cli':
                    com += "udp" + "=" + str(iface.addr) + ":" + str(iface.port) + ",localaddr=" + str(iface.laddr) + ':' +  str(iface.lport)
            elif iface.proto == 'tcp': 
                proto = None
                if iface.type == 'srv':
                    proto = 'listen'
                elif iface.type == 'cli':
                    proto = 'connect'
                com += str(proto) + "=" + str(iface.addr) + ":" + str(iface.port)
            elif iface.proto == 'slirp':
                restrict = "off"
                if iface.restrict:
                    restrict = "on"
                com = "host_net_add user " + "name=socket." + str(iface.id) + ",vlan=" + str(iface.id)
                if iface.net:
                    com += ",net=" + str(iface.net) 
                com += ",restrict=" + restrict
                for fw in iface.forwards:
                    com += "\n" + "hostfwd_add " + str(iface.id) + " socket." + str(iface.id) + " " + str(fw.proto) + ":" + str(fw.haddr) + ":" + str(fw.hport) + "-" + str(fw.gaddr) + ":" + str(fw.gport)
            elif iface.proto == 'vde':
                com = "host_net_add vde name=socket." + str(iface.id) + ",vlan=" + str(iface.id) + ",sock=" + str(iface.addr)
            elif iface.proto == 'tap':
                com = "host_net_add tap name=socket." + str(iface.id) + ",vlan=" + str(iface.id) + ",ifname=" + str(iface.addr) + ",script=" + str(NemuVar.tapup) + ",downscript=" + str(NemuVar.tapdown)
	    else:
		raise NemuError(str(iface.proto) + ' is not supported by VNode')
            self.send(com)
           
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
            self.send("host_net_remove " + str(iface.id) + " socket." + str(iface.id))

    def confiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Sets interface properties of <id>'''
        raise NemuError("Interface properties are not available for VNode")
    
    def unconfiface(self, id, direction, state, vlan, bandwidth, delay, jitter, ber, qsize):
        '''Unsets interface properties of <id>'''
        raise NemuError("Interface properties are not available for VNode")

    def running(self):
        '''Indicates if the vnode is active'''
        return not self.proc == None and (self.proc.pid in NemuProc.procl)

    def start(self):
        '''Starts the vnode'''
        if self.running():
            return
        printc('Launching the VNode ' + str(self.name) , 'blue')
        self.proc = popenproc(self.qstr(), logfile=self.logfile)
        if self.running():
            for i in self.ifaces:
                self.addlink(i.id)
                self.dumplink(i.id)
                if i.type == 'srv' or i.type == 'peer':
                    self.setlink(i.id)
        printok()
        
    def stop(self):
        '''Stops the vnode'''
        if self.running():
            printc('Stopping the VNode ' + str(self.name) , 'cyan')
            pid = self.proc.pid
            for i in self.ifaces:
                self.unsetlink(i.id)
            self.send("quit")
            killprocl(pid)

    def reboot(self):
        '''Restarts the vnode'''
        if self.running():
            printc('Rebooting the VNode ' + str(self.name) , 'cyan')
            self.send("system_reset")

    def send(self, line):
        '''Write in the vnode stdin'''
        if self.running():
            try:
                self.proc.stdin.write(str(line) + "\n")
                printlog(str(line).rstrip('\n'), logf=self.infile)
            except (AttributeError, IOError, ValueError):
                printc("Cannot communicate with " + str(self.name), 'magenta')
                pass

    def delete(self):
        '''Deletes the vnode'''
        self.stop()
        if os.path.isdir(self.vard):
            shutil.rmtree(self.vard, True)
        for i in self.ifaces:
            i.delete()
        del VNode.set[self.name]
        del NemuNameSpace.set[self.name]
                
    def qstr_host(self):
        '''QEMU host string'''
        com = self.qemu + ' -name ' + str(self.name) + ' -monitor stdio '
        for i in self.hds:
            if i == None:
                continue
            if i.type == 'virtio':
                com += i.qstr() + ' '
            else:
                com += '-hd' + chr(ord('a') + self.hds.index(i)) + ' ' + i.qstr() + ' '
        return com + ' '

    def qstr_opt(self):
        '''QEMU options string'''
        com = ''
        keys = sorted(self.opts.keys())
        for i in keys:
            if isinstance(self.opts[i], list):
                for j in self.opts[i]:
                    com += '-' + str(i) + ' ' + str(j) + ' '
            else:
                com += '-' + str(i) + ' '
                if not self.opts[i] == None:
                    com += str(self.opts[i]) + ' '
        return com + ' '

    def qstr_net(self):
        '''QEMU network string'''
        com = '-net none'
        #com = ''
        #for i in self.ifaces:
        #    com += '-net nic,vlan=' + str(i.id) + ',macaddr=' + str(i.hw)
        #    if i.model:
        #        com += ',model=' + str(i.model)
        #    com += ' '
        #if len(self.ifaces) == 0:
        #    com += '-net none'
        return com + ' '

    def qstr(self):
        '''QEMU host string'''
        return self.qstr_host() + self.qstr_net() + self.qstr_opt()

    def gstr(self):
        '''Graph description'''
        com = str(self.name) + ' [shape=polygon,sides=4,peripheries=2,color='
        if self.running():
            com += 'green'
        else:
            com += 'lightgray'
        com += ',style=filled,label="' + str(self.__class__.__name__) + '['+ str(self.name) + ']"];'
        return com

    def estr_hds(self):
        '''VFs format string'''
        com = 'hds=['
        for i in self.hds:
            if i == None:
                continue
            com += str(i)
            if not self.hds.index(i) == (len(self.hds)-1):
                com += ','
        com += ']'
        return com

    def estr_net(self):
        '''Network format string'''
        com = 'nics=['
        nifaces = len(self.ifaces)
        for i in self.ifaces:
            com += repr(i)
            nifaces -= 1
            if not nifaces == 0:
                com += ','
        com += ']'
        return com

    def __str__(self):
        com = self.__class__.__name__ + '('
        com += 'name=' + printstr(self.name)
        com += ',' + self.estr_hds()
        com += ',' + self.estr_net()
        com += ',qemu=' + printstr(self.qemu)
        com += ',**' + printstr(self.opts)
        com += ')'
        return com

    def __repr__(self):
        return str(self)
